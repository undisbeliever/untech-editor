#include "compiler.h"
#include "combinesmalltilesets.h"
#include "version.h"
#include "models/metasprite-common/framesetexportorder.h"
#include "models/metasprite-common/limits.h"
#include "models/snes/palette.hpp"
#include <algorithm>
#include <climits>
#include <set>

using namespace UnTech::MetaSpriteCompiler;
namespace MS = UnTech::MetaSprite;
namespace MSC = UnTech::MetaSpriteCommon;

inline bool Compiler::AnimationListEntry::operator==(const Compiler::AnimationListEntry& o) const
{
    return animation == o.animation && hFlip == o.hFlip && vFlip == o.vFlip;
}

inline bool Compiler::AnimationListEntry::operator<(const Compiler::AnimationListEntry& o) const
{
    return std::tie(animation, hFlip, vFlip) < std::tie(o.animation, o.hFlip, o.vFlip);
}

inline bool Compiler::FrameListEntry::operator==(const Compiler::FrameListEntry& o) const
{
    return frame == o.frame && hFlip == o.hFlip && vFlip == o.vFlip;
}

inline bool Compiler::FrameListEntry::operator<(const Compiler::FrameListEntry& o) const
{
    return std::tie(frame, hFlip, vFlip) < std::tie(o.frame, o.hFlip, o.vFlip);
}

// ::TODO generate debug file - containing frame/frameset names::

Compiler::Compiler(unsigned tilesetBlockSize)
    : _frameSetData("FSD", "MS_FrameSetData")
    , _frameSetList("FSL", "MS_FrameSetList", "FSD")
    , _frameData("FD", "MS_FrameData")
    , _frameList("FL", "MS_FrameList", "FD")
    , _animationData("AD", "MS_AnimationData")
    , _animationList("AL", "MS_AnimationList", "AD")
    , _paletteData("PD", "MS_PaletteData")
    , _paletteList("PL", "MS_PaletteList", "PD")
    , _tileData("TB", "MS_TileBlock", tilesetBlockSize)
    , _tilesetData("TS", "DMA_Tile16Data")
    , _frameObjectData("FO", "MS_FrameObjectsData", true)
    , _tileHitboxData("TC", "MS_TileHitboxData", true)
    , _entityHitboxData("EH", "MS_EntityHitboxData", true)
    , _actionPointData("AP", "MS_ActionPointsData", true)
    , _frameSetReferences()
    , _exportOrderDocuments()
    , _errors()
    , _warnings()
{
}

void Compiler::writeToIncFile(std::ostream& out) const
{
    out << "scope MetaSprite {\n"
           "scope Data {\n"
           "\n"
        << "constant EDITOR_VERSION(" << UNTECH_VERSION_INT << ")\n";

    _animationData.writeToIncFile(out);
    _animationList.writeToIncFile(out);

    _paletteData.writeToIncFile(out);
    _paletteList.writeToIncFile(out);

    _tileData.writeToIncFile(out);
    _tilesetData.writeToIncFile(out);

    _frameObjectData.writeToIncFile(out);
    _tileHitboxData.writeToIncFile(out);
    _actionPointData.writeToIncFile(out);
    _entityHitboxData.writeToIncFile(out);

    _frameData.writeToIncFile(out);
    _frameList.writeToIncFile(out);

    _frameSetData.writeToIncFile(out);
    _frameSetList.writeToIncFile(out);
    out << "constant FrameSetListCount((pc() - FSL)/2)\n";

    out << "}\n"
           "}\n";
}

void Compiler::writeToReferencesFile(std::ostream& out) const
{
    out << "scope MSFS {\n";

    for (unsigned i = 0; i < _frameSetReferences.size(); i++) {
        const auto& r = _frameSetReferences[i];

        if (!r.isNull) {
            out << "\tconstant " << r.name << "(" << i << ")\n";
            out << "\tdefine " << r.name << ".type(" << r.exportOrderName << ")\n";
        }
    }

    out << "}\n"
           "scope MSEO {\n";

    for (const auto& eoDoc : _exportOrderDocuments) {
        const auto& eo = eoDoc->exportOrder();

        out << "\tscope " << eo.name() << " {\n";

        if (eo.stillFrames().size() > 0) {
            unsigned id = 0;
            out << "\t\tscope Frames {\n";
            for (const auto& sfIt : eo.stillFrames()) {
                out << "\t\t\tconstant " << sfIt.first << "(" << id << ")\n";
                id++;
            }
            out << "\t\t}\n";
        }
        if (eo.animations().size() > 0) {
            unsigned id = 0;
            out << "\t\tscope Animations {\n";
            for (const auto& sfIt : eo.animations()) {
                out << "\t\t\tconstant " << sfIt.first << "(" << id << ")\n";
                id++;
            }
            out << "\t\t}\n";
        }
        out << "\t}\n";
    }

    out << "}\n";
}

/*
 * ANIMATIONS
 * ==========
 */
inline std::vector<Compiler::AnimationListEntry>
Compiler::generateAnimationList(const MS::FrameSet& frameSet)
{
    assert(frameSet.exportOrderDocument() != nullptr);

    std::vector<AnimationListEntry> ret;

    const auto& exportOrder = frameSet.exportOrderDocument()->exportOrder();

    ret.reserve(exportOrder.animations().size());

    for (const auto& sfIt : exportOrder.animations()) {
        const std::string& aname = sfIt.first;

        MSC::Animation* ani = frameSet.animations().getPtr(aname);

        if (ani != nullptr) {
            ret.push_back({ ani, false, false });
        }
        else {
            bool success = false;

            for (const auto& alt : sfIt.second.alternativeNames()) {
                MSC::Animation* altAni = frameSet.animations().getPtr(alt.name());

                if (altAni != nullptr) {
                    ret.push_back({ altAni, alt.hFlip(), alt.vFlip() });

                    success = true;
                    break;
                }
            }

            if (success == false) {
                throw std::runtime_error("Cannot find animation " + aname);
            }
        }
    }

    return ret;
}

inline uint32_t
Compiler::processAnimation(const AnimationListEntry& aniEntry,
                           const MS::FrameSet& frameSet,
                           const std::map<const FrameListEntry, unsigned>& frameMap,
                           const std::map<const AnimationListEntry, unsigned>& animationMap)
{
    typedef MSC::AnimationBytecode::Enum BC;

    assert(aniEntry.animation != nullptr);
    const MSC::Animation& animation = *aniEntry.animation;

    std::vector<uint8_t> data;
    data.reserve(animation.instructions().size() * 3);

    const auto& instructions = animation.instructions();
    for (auto instIt = instructions.begin(); instIt != instructions.end(); ++instIt) {
        const auto& inst = *instIt;

        data.push_back(inst.operation().engineValue());

        switch (inst.operation().value()) {
        case BC::GOTO_ANIMATION: {
            const MSC::Animation* a = frameSet.animations().getPtr(inst.gotoLabel());

            // Find animation Id
            if (a == nullptr) {
                throw std::runtime_error("Cannot find animation " + inst.gotoLabel());
            }
            auto it = animationMap.find({ a, aniEntry.hFlip, aniEntry.vFlip });
            if (it == animationMap.end()) {
                it = animationMap.find({ a, false, false });
            }
            if (it == animationMap.end()) {
                throw std::runtime_error("Cannot find animation " + inst.gotoLabel());
            }
            data.push_back(it->second);

            break;
        }

        case BC::GOTO_OFFSET: {
            // will always succeed because inst is valid.

            int p = inst.parameter();
            UnTech::int_ms8_t offset = 0;

            if (p < 0) {
                assert(std::distance(instructions.begin(), instIt) >= -p);
                assert(instIt + (p + 1) != instructions.begin());

                for (auto gotoIt = instIt + p; gotoIt != instIt; ++gotoIt) {
                    offset -= (*gotoIt)->operation().instructionSize();
                }
            }
            else {
                assert(std::distance(instIt, instructions.end()) >= p);
                assert(instIt + p != instructions.end());

                for (auto gotoIt = instIt + p; gotoIt != instIt; --gotoIt) {
                    offset += (*gotoIt)->operation().instructionSize();
                }
            }

            data.push_back(offset.romData());

            break;
        }

        case BC::SET_FRAME_AND_WAIT_FRAMES:
        case BC::SET_FRAME_AND_WAIT_TIME:
        case BC::SET_FRAME_AND_WAIT_XVECL:
        case BC::SET_FRAME_AND_WAIT_YVECL: {
            const std::string& fname = inst.frame().frameName;

            FrameListEntry f = { frameSet.frames().getPtr(fname),
                                 static_cast<bool>(inst.frame().hFlip ^ aniEntry.hFlip),
                                 static_cast<bool>(inst.frame().vFlip ^ aniEntry.vFlip) };

            data.push_back(frameMap.at(f));

            assert(inst.parameter() >= 0 && inst.parameter() < 256);
            data.push_back(inst.parameter());

            break;
        }

        case BC::STOP:
        case BC::GOTO_START:
            break;
        }
    }

    return _animationData.addData(data).offset;
}

inline RomOffsetPtr
Compiler::processAnimationList(const MS::FrameSet& frameSet,
                               const std::vector<AnimationListEntry>& animationList,
                               const std::vector<FrameListEntry>& frameList)
{
    if (animationList.size() >= 256) {
        throw std::runtime_error("Too many animations");
    }

    std::map<const FrameListEntry, unsigned> frameMap;
    for (unsigned i = 0; i < frameList.size(); i++) {
        frameMap[frameList[i]] = i;
    }

    std::map<const AnimationListEntry, unsigned> animationMap;
    for (unsigned i = 0; i < animationList.size(); i++) {
        animationMap[animationList[i]] = i;
    }

    std::vector<uint32_t> animationOffsets;
    animationOffsets.reserve(animationList.size());

    for (const auto& ani : animationList) {
        assert(ani.animation != nullptr);
        uint32_t ao = ~0;

        if (ani.animation->isValid()) {
            ao = processAnimation(ani, frameSet, frameMap, animationMap);
        }
        else {
            addError(*ani.animation, "animation is invalid");
        }

        animationOffsets.push_back(ao);
    }

    return _animationList.getOrInsertTable(animationOffsets);
}

/*
 * FRAME LIST
 * ==========
 */

inline std::vector<Compiler::FrameListEntry> Compiler::generateFrameList(
    const MS::FrameSet& frameSet,
    const std::vector<AnimationListEntry>& animationList)
{
    assert(frameSet.exportOrderDocument() != nullptr);

    std::vector<FrameListEntry> ret;

    const auto& frames = frameSet.frames();
    const auto& exportOrder = frameSet.exportOrderDocument()->exportOrder();

    ret.reserve(exportOrder.stillFrames().size());

    for (const auto& sfIt : exportOrder.stillFrames()) {
        const std::string& fname = sfIt.first;

        MS::Frame* f = frames.getPtr(fname);

        if (f != nullptr) {
            ret.push_back({ f, false, false });
        }
        else {
            bool success = false;

            for (const auto& alt : sfIt.second.alternativeNames()) {
                MS::Frame* af = frames.getPtr(alt.name());

                if (af != nullptr) {
                    ret.push_back({ af, alt.hFlip(), alt.vFlip() });

                    success = true;
                    break;
                }
            }

            if (success == false) {
                throw std::runtime_error("Cannot find frame " + fname);
            }
        }
    }

    // Add frames from animation.
    // Ensure that the frames added are unique.
    for (const AnimationListEntry& ani : animationList) {
        for (const auto& inst : ani.animation->instructions()) {
            const auto& op = inst.operation();
            if (op.usesFrame()) {
                const std::string& fname = inst.frame().frameName;

                FrameListEntry e = { frames.getPtr(fname),
                                     static_cast<bool>(inst.frame().hFlip ^ ani.hFlip),
                                     static_cast<bool>(inst.frame().vFlip ^ ani.vFlip) };

                if (e.frame == nullptr) {
                    throw std::runtime_error("Cannot find frame " + fname);
                }

                auto it = std::find(ret.begin(), ret.end(), e);
                if (it == ret.end()) {
                    // new entry
                    ret.push_back(e);
                }
            }
        }
    }

    return ret;
}

/*
 * TILESETS
 * ========
 */

/*
 * Increments the `charattr` position by one 16x16 tile,
 * handling the tilesetSpitPoint as required.
 */
struct CharAttrPos {
    const static unsigned CHARATTR_SIZE_LARGE = 0x0200;
    const static unsigned CHARATTR_TILE_ID_MASK = 0x001F;
    const static unsigned CHARATTR_BLOCK_TWO = 0x0020;
    const static unsigned CHARATTR_HFLIP = 0x4000;
    const static unsigned CHARATTR_VFLIP = 0x8000;
    const static uint16_t SMALL_TILE_OFFSETS[4];

    CharAttrPos(unsigned tilesetSplitPoint)
        : value(0)
        , charSplitPoint(tilesetSplitPoint * 2)
    {
    }

    uint16_t largeCharAttr() const { return value | CHARATTR_SIZE_LARGE; }
    uint16_t smallCharAttr(unsigned i) const { return value | SMALL_TILE_OFFSETS[i]; }

    void inc()
    {
        value += 2;

        if (value == charSplitPoint) {
            value = CHARATTR_BLOCK_TWO;
        }
    }

    uint16_t value;
    const uint_fast8_t charSplitPoint;
};

const uint16_t CharAttrPos::SMALL_TILE_OFFSETS[4] = { 0x0000, 0x0001, 0x0010, 0x0011 };

void Compiler::buildTileset(FrameTileset& tileset,
                            const MS::FrameSet& frameSet,
                            const MSC::TilesetType& tilesetType,
                            const std::set<unsigned>& largeTiles,
                            const std::set<std::array<unsigned, 4>>& smallTiles)
{
    unsigned tileCount = largeTiles.size() + smallTiles.size();
    assert(tileCount > 0);
    assert(tileCount <= tilesetType.nTiles());

    RomIncItem tilesetTable;
    tilesetTable.addField(RomIncItem::BYTE, tileCount);

    // Process Tiles
    {
        auto addTile = [&](const Snes::Tile4bpp16px& tile) mutable {
            auto a = _tileData.addLargeTile(tile);
            tilesetTable.addTilePtr(a.addr);

            return a;
        };

        // Process Large Tiles
        CharAttrPos charAttrPos(tilesetType.tilesetSplitPoint());

        for (unsigned tId : largeTiles) {
            const Snes::Tile4bpp16px& tile = frameSet.largeTileset().tile(tId);

            RomTileData::Accessor a = addTile(tile);

            uint16_t charAttr = charAttrPos.largeCharAttr();
            if (a.hFlip) {
                charAttr |= CharAttrPos::CHARATTR_HFLIP;
            }
            if (a.vFlip) {
                charAttr |= CharAttrPos::CHARATTR_VFLIP;
            }
            tileset.largeTilesetMap.emplace(tId, charAttr);

            charAttrPos.inc();
        }

        // Process Small Tiles
        for (auto tileIds : smallTiles) {
            std::array<Snes::Tile4bpp8px, 4> tile = {};

            for (unsigned i = 0; i < 4; i++) {
                unsigned tId = tileIds[i];
                if (tId < frameSet.smallTileset().size()) {
                    tile[i] = frameSet.smallTileset().tile(tId);
                }
            }
            RomTileData::Accessor a = addTile(tile);

            for (unsigned i = 0; i < 4; i++) {
                unsigned tId = tileIds[i];
                uint16_t charAttr = charAttrPos.smallCharAttr(i);

                if (a.hFlip) {
                    charAttr |= CharAttrPos::CHARATTR_HFLIP;
                }
                if (a.vFlip) {
                    charAttr |= CharAttrPos::CHARATTR_VFLIP;
                }
                tileset.smallTilesetMap.emplace(tId, charAttr);
            }

            charAttrPos.inc();
        }
    }

    // Store the DMA data and tileset
    tileset.tilesetOffset = _tilesetData.addData(tilesetTable);
}

void Compiler::buildTileset(FrameTileset& tileset,
                            const MS::FrameSet& frameSet,
                            const MSC::TilesetType& tilesetType,
                            const std::set<unsigned>& largeTiles,
                            const std::set<unsigned>& smallTiles)
{
    std::set<std::array<unsigned, 4>> smallTilesCombined;

    auto it = smallTiles.begin();
    while (it != smallTiles.end()) {
        std::array<unsigned, 4> st = { { UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX } };

        for (unsigned i = 0; i < st.size() && it != smallTiles.end(); i++) {
            st[i] = *it;
            ++it;
        }
        smallTilesCombined.insert(st);
    }

    buildTileset(tileset, frameSet, tilesetType, largeTiles, smallTilesCombined);
}

inline Compiler::FrameTilesetList
Compiler::generateDynamicTilesets(const MetaSprite::FrameSet& frameSet,
                                  const MetaSpriteCommon::TilesetType& tilesetType,
                                  const TileGraph_t& largeTileGraph,
                                  const TileGraph_t& smallTileGraph)
{
    auto smallTileMap = combineSmallTilesets(smallTileGraph);

    // convert to a more use-able format
    struct FrameTileset {
        std::vector<const MetaSprite::Frame*> frames;
        std::set<unsigned> originalSmallTiles;
        std::set<std::array<unsigned, 4>> smallTiles;
        std::set<unsigned> largeTiles;

        unsigned nTiles() const
        {
            return largeTiles.size() + smallTiles.size();
        }

        unsigned nTilesUnoptimized() const
        {
            return largeTiles.size() + (originalSmallTiles.size() + 3) / 4;
        }
    };

    std::vector<FrameTileset> ftData;
    {
        std::map<const MetaSprite::Frame*, FrameTileset> ft;

        for (const auto& it : largeTileGraph) {
            for (const auto& f : it.second) {
                ft[f].largeTiles.insert(it.first);
            }
        }
        for (const auto& it : smallTileGraph) {
            for (const auto& f : it.second) {
                ft[f].smallTiles.insert(smallTileMap[it.first]);
                ft[f].originalSmallTiles.insert(it.first);
            }
        }

        for (auto& fIt : ft) {
            const auto& data = fIt.second;

            auto dup = std::find_if(ftData.begin(), ftData.end(), [data](const auto& t) {
                return t.originalSmallTiles == data.originalSmallTiles
                       && t.largeTiles == data.largeTiles;
            });

            if (dup != ftData.end()) {
                dup->frames.push_back(fIt.first);
            }
            else {
                ftData.emplace_back(data);
                ftData.back().frames.push_back(fIt.first);
            }
        }
    }

    // Build tilesets
    {
        FrameTilesetList ret;
        ret.tilesetType = tilesetType;

        for (const auto& ft : ftData) {
            ret.tilesets.emplace_back();
            auto& tileset = ret.tilesets.back();

            if (ft.nTiles() <= tilesetType.nTiles()) {
                buildTileset(tileset, frameSet, tilesetType, ft.largeTiles, ft.smallTiles);
            }
            else if (ft.nTilesUnoptimized() <= tilesetType.nTiles()) {
                addWarning(frameSet, "Could not optimally combine tileset");
                buildTileset(tileset, frameSet, tilesetType, ft.largeTiles, ft.originalSmallTiles);
            }
            else {
                const std::string fn = frameSet.frames().getName(ft.frames.front()).value();
                throw std::runtime_error("Too many tiles in frame " + fn);
            }

            for (const MS::Frame* frame : ft.frames) {
                ret.frameMap.emplace(frame, tileset);
            }
        }

        return ret;
    }
}

inline Compiler::FrameTilesetList
Compiler::generateFixedTileset(const MetaSprite::FrameSet& frameSet,
                               const MetaSpriteCommon::TilesetType& tilesetType,
                               const TileGraph_t& largeTileGraph,
                               const TileGraph_t& smallTileGraph)
{
    // Duplicates of large tiles are handled by the RomTileData class
    // Duplicates of small tiles can not be easily processed, and will be ignored.
    std::set<unsigned> largeTiles;
    std::set<unsigned> smallTiles;
    {
        for (const auto& it : largeTileGraph) {
            largeTiles.insert(it.first);
        }
        for (const auto& it : smallTileGraph) {
            smallTiles.insert(it.first);
        }
    }

    // Build and store tileset
    {
        FrameTilesetList ret;
        ret.tilesetType = tilesetType;

        ret.tilesets.emplace_back();
        FrameTileset& tileset = ret.tilesets.back();
        buildTileset(tileset, frameSet, tilesetType, largeTiles, smallTiles);

        for (const auto& fsIt : frameSet.frames()) {
            ret.frameMap.emplace(&fsIt.second, tileset);
        }

        return ret;
    }
}

inline Compiler::FrameTilesetList
Compiler::generateTilesetList(const MS::FrameSet& frameSet,
                              const std::vector<FrameListEntry>& frameList)
{
    TileGraph_t largeTileGraph;
    TileGraph_t smallTileGraph;

    // Build a graph of the tiles and the frames that use them.
    for (const auto& fle : frameList) {
        if (fle.frame != nullptr) {
            for (const auto& obj : fle.frame->objects()) {

                if (obj.size() == MS::FrameObject::ObjectSize::LARGE) {
                    largeTileGraph[obj.tileId()].emplace_back(fle.frame);
                }
                else {
                    smallTileGraph[obj.tileId()].emplace_back(fle.frame);
                }
            }
        }
    }

    // verify tileIds are valid
    {
        for (const auto& it : smallTileGraph) {
            unsigned t = it.first;
            if (t >= frameSet.smallTileset().size()) {
                throw std::runtime_error("Invalid small tileId " + std::to_string(t));
            }
        }
        for (const auto& it : largeTileGraph) {
            unsigned t = it.first;
            if (t >= frameSet.largeTileset().size()) {
                throw std::runtime_error("Invalid large tileId " + std::to_string(t));
            }
        }
    }

    unsigned tilesetSize = largeTileGraph.size() + (smallTileGraph.size() + 3) / 4;
    auto tilesetType = frameSet.tilesetType();

    if (tilesetSize == 0) {
        throw std::runtime_error("No tiles in tileset");
    }

    if (tilesetSize <= tilesetType.nTiles() || tilesetType.isFixed()) {
        // All of the tiles used in the frameset
        if (tilesetType.isFixed() == false) {
            addWarning(frameSet, "Tileset can be fixed, making it so.");
        }

        // resize tileset if necessary
        auto smallestType = MSC::TilesetType::smallestFixedTileset(tilesetSize);

        if (tilesetType.nTiles() != smallestType.nTiles()) {
            addWarning(frameSet, std::string("TilesetType shrunk to ") + smallestType.string());

            tilesetType = smallestType;
        }

        return generateFixedTileset(frameSet, tilesetType, largeTileGraph, smallTileGraph);
    }
    else {
        return generateDynamicTilesets(frameSet, tilesetType, largeTileGraph, smallTileGraph);
    }
}

/*
 * PALETTE
 * =======
 */
inline RomOffsetPtr Compiler::processPalette(const MS::FrameSet& frameSet)
{
    // Some framesets can have the same palette, will check for duplicates
    const auto& palettes = frameSet.palettes();

    if (palettes.size() == 0) {
        throw std::runtime_error("No Palettes in Frameset");
    }
    if (palettes.size() > 255) {
        throw std::runtime_error("Too many palettes (max 255)");
    }

    std::vector<uint32_t> offsets;
    offsets.reserve(palettes.size());

    for (const auto& palette : palettes) {
        std::vector<uint8_t> paletteData = palette.paletteData();

        // Remove transparent color, saves 2 bytes in ROM.
        paletteData.erase(paletteData.begin(), paletteData.begin() + 2);

        offsets.emplace_back(_paletteData.addData(paletteData).offset);
    }

    return _paletteList.getOrInsertTable(offsets);
}

/*
 * FRAMES
 * ======
 */

inline RomOffsetPtr Compiler::processFrameObjects(const MS::FrameObject::list_t& objects,
                                                  const FrameTileset& tileset)
{
    const static uint16_t V_FLIP = 0x8000;
    const static uint16_t H_FLIP = 0x4000;
    const static uint16_t ORDER_MASK = 0x03;
    const static unsigned ORDER_SHIFT = 12;

    if (objects.size() == 0) {
        return RomOffsetPtr();
    }

    if (objects.size() > MSC::MAX_FRAME_OBJECTS) {
        throw std::runtime_error("Too many frame objects");
    }

    std::vector<uint8_t> romData;
    romData.reserve(1 + 4 * objects.size());

    romData.push_back(objects.size()); // count

    for (const MS::FrameObject& obj : objects) {
        const ms8point loc = obj.location();

        uint16_t charAttr;

        if (obj.size() == MS::FrameObject::ObjectSize::SMALL) {
            charAttr = tileset.smallTilesetMap.at(obj.tileId());
        }
        else {
            charAttr = tileset.largeTilesetMap.at(obj.tileId());
        }

        charAttr |= (obj.order() & ORDER_MASK) << ORDER_SHIFT;

        if (obj.hFlip()) {
            charAttr ^= H_FLIP;
        }

        if (obj.vFlip()) {
            charAttr ^= V_FLIP;
        }

        romData.push_back(loc.x.romData());        // Objects::xOffset
        romData.push_back(loc.y.romData());        // Objects::yOffset
        romData.push_back(charAttr & 0xFF);        // Objects::char
        romData.push_back((charAttr >> 8) & 0xFF); // Objects::attr
    }

    return _frameObjectData.addData(romData);
}

inline RomOffsetPtr Compiler::processEntityHitboxes(const MS::EntityHitbox::list_t& entityHitboxes)
{
    if (entityHitboxes.size() == 0) {
        return RomOffsetPtr();
    }

    if (entityHitboxes.size() > MSC::MAX_ENTITY_HITBOXES) {
        throw std::runtime_error("Too many entity hitboxes");
    }

    unsigned count = entityHitboxes.size();
    unsigned dataSize = 7 + 7 * count;

    // a Hitbox with a single aabb is a special case.
    if (count == 1) {
        count = 0;
        dataSize = 7 + 1;
    }

    std::vector<uint8_t> romData;
    romData.reserve(dataSize);

    ms8rect outerAabb;
    {
        // calculate outer AABB.
        for (const MS::EntityHitbox& eh : entityHitboxes) {
            outerAabb.extend(eh.aabb());
        }

        if (outerAabb.width == 0 || outerAabb.height == 0) {
            throw std::runtime_error("Entity Hitbox aabb has no size");
        }
    }

    romData.push_back(count); // count

    romData.push_back(outerAabb.x.romData()); // Outer::xOffset
    romData.push_back(outerAabb.y.romData()); // Outer::yOffset
    romData.push_back(outerAabb.width);       // Outer::width
    romData.push_back(0);                     // Outer::width high byte
    romData.push_back(outerAabb.height);      // Outer::height
    romData.push_back(0);                     // Outer::height high byte

    if (count > 0) {
        for (const MS::EntityHitbox& eh : entityHitboxes) {
            const ms8rect& innerAabb = eh.aabb();

            if (innerAabb.width == 0 || innerAabb.height == 0) {
                throw std::runtime_error("Entity Hitbox aabb has no size");
            }

            romData.push_back(eh.hitboxType().romValue()); // Inner:type
            romData.push_back(innerAabb.x.romData());      // Inner::xOffset
            romData.push_back(innerAabb.y.romData());      // Inner::yOffset
            romData.push_back(innerAabb.width);            // Inner::width
            romData.push_back(0);                          // Inner::width high byte
            romData.push_back(innerAabb.height);           // Inner::height
            romData.push_back(0);                          // Inner::height high byte
        }
    }
    else {
        const MS::EntityHitbox& eh = entityHitboxes.at(0);
        romData.push_back(eh.hitboxType().romValue()); // SingleHitbox::type
    }

    return _entityHitboxData.addData(romData);
}

inline RomOffsetPtr Compiler::processTileCollisionHitbox(const MS::Frame& frame)
{
    if (frame.solid() == false) {
        return RomOffsetPtr();
    }

    const ms8rect& aabb = frame.tileHitbox();

    if (aabb.width == 0 || aabb.height == 0) {
        throw std::runtime_error("Tileset Hitbox aabb has no size");
    }

    std::vector<uint8_t> romData(4);

    romData.push_back(aabb.x.romData()); // xOffset
    romData.push_back(aabb.y.romData()); // yOffset
    romData.push_back(aabb.width);       // width
    romData.push_back(aabb.height);      // height

    return _tileHitboxData.addData(romData);
}

inline RomOffsetPtr Compiler::processActionPoints(const MS::ActionPoint::list_t& actionPoints)
{
    if (actionPoints.size() == 0) {
        return RomOffsetPtr();
    }

    if (actionPoints.size() > MSC::MAX_ACTION_POINTS) {
        throw std::runtime_error("Too many action points");
    }

    std::vector<uint8_t> romData;
    romData.reserve(3 * actionPoints.size() + 1);

    for (const MS::ActionPoint& ap : actionPoints) {
        if (ap.parameter() == 0) {
            throw std::runtime_error("Action Point parameter cannot be zero");
        }

        const ms8point loc = ap.location();

        romData.push_back(ap.parameter());  // Point::parameter
        romData.push_back(loc.x.romData()); // Point::xOffset
        romData.push_back(loc.y.romData()); // Point::yOffset
    }

    romData.push_back(0); // null terminator

    return _actionPointData.addData(romData);
}

uint32_t Compiler::processFrame(const MS::Frame& frame, const FrameTileset& tileset)
{
    RomOffsetPtr frameObjects = processFrameObjects(frame.objects(), tileset);
    RomOffsetPtr enityHitbox = processEntityHitboxes(frame.entityHitboxes());
    RomOffsetPtr tileHitboxHitbox = processTileCollisionHitbox(frame);
    RomOffsetPtr actionPoints = processActionPoints(frame.actionPoints());

    RomIncItem data;
    data.addAddr(frameObjects);
    data.addAddr(enityHitbox);
    data.addAddr(tileHitboxHitbox);
    data.addAddr(actionPoints);
    data.addAddr(tileset.tilesetOffset);

    return _frameData.addData(data).offset;
}

inline RomOffsetPtr Compiler::processFrameList(const std::vector<FrameListEntry>& frameList,
                                               const FrameTilesetList& tilesets)
{
    if (frameList.size() >= 256) {
        throw std::runtime_error("Too many frames");
    }

    const uint32_t NULL_OFFSET = ~0;

    std::vector<uint32_t> frameOffsets;
    frameOffsets.reserve(frameList.size());

    for (const auto& fle : frameList) {
        if (fle.frame != nullptr) {
            try {
                uint32_t fo = NULL_OFFSET;
                const FrameTileset& frameTileset = tilesets.frameMap.at(fle.frame);

                if (fle.hFlip == false && fle.vFlip == false) {
                    fo = processFrame(*fle.frame, frameTileset);
                }
                else {
                    auto flippedFrame = fle.frame->flip(fle.hFlip, fle.vFlip);

                    fo = processFrame(*flippedFrame, frameTileset);
                }

                frameOffsets.push_back(fo);
            }
            catch (const std::exception& ex) {
                addError(*fle.frame, ex.what());

                frameOffsets.push_back(NULL_OFFSET);
            }
        }
        else {
            frameOffsets.push_back(NULL_OFFSET);
        }
    }

    return _frameList.getOrInsertTable(frameOffsets);
}

/*
 * FRAMESET
 * ========
 */

void Compiler::processNullFrameSet()
{
    _frameSetList.addNull();
    _frameSetReferences.emplace_back();
}

void Compiler::processFrameSet(const MS::FrameSet& frameSet)
{
    if (frameSet.exportOrderDocument() == nullptr) {
        addError(frameSet, "No frameset export order");
        return processNullFrameSet();
    }

    try {
        std::vector<AnimationListEntry> animationList = generateAnimationList(frameSet);
        std::vector<FrameListEntry> frameList = generateFrameList(frameSet, animationList);

        FrameTilesetList tilesets = generateTilesetList(frameSet, frameList);

        RomOffsetPtr fsPalettes = processPalette(frameSet);
        RomOffsetPtr fsFrames = processFrameList(frameList, tilesets);
        RomOffsetPtr fsAnimations = processAnimationList(frameSet, animationList, frameList);

        // FRAMESET DATA
        // -------------
        RomIncItem frameSetItem;

        unsigned nPalettes = frameSet.palettes().size();

        frameSetItem.addIndex(fsPalettes);                  // paletteTable
        frameSetItem.addField(RomIncItem::BYTE, nPalettes); // nPalettes

        // tileset
        if (tilesets.tilesets.size() == 1) {
            frameSetItem.addAddr(tilesets.tilesets.front().tilesetOffset);
        }
        else {
            frameSetItem.addField(RomIncItem::ADDR, 0);
        }

        frameSetItem.addField(RomIncItem::BYTE, tilesets.tilesetType.romValue()); // tilesetType
        frameSetItem.addIndex(fsFrames);                                          // frameTable
        frameSetItem.addField(RomIncItem::BYTE, frameList.size());                // nFrames
        frameSetItem.addIndex(fsAnimations);                                      // animationsTable
        frameSetItem.addField(RomIncItem::BYTE, animationList.size());            // nAnimations

        RomOffsetPtr ptr = _frameSetData.addData(frameSetItem);
        _frameSetList.addOffset(ptr.offset);

        // add to references
        _frameSetReferences.emplace_back(frameSet.name(),
                                         frameSet.exportOrderDocument()->exportOrder().name());
        _exportOrderDocuments.insert(frameSet.exportOrderDocument());
    }
    catch (const std::exception& ex) {
        addError(frameSet, ex.what());
        processNullFrameSet();
    }
}

/*
 * ERROR HANDLING
 * ==============
 */

void Compiler::addError(const std::string& message)
{
    _errors.push_back(message);
}

void Compiler::addError(const MS::FrameSet& frameSet, const std::string& message)
{
    std::stringstream out;

    out << frameSet.name()
        << ": "
        << message;

    _errors.push_back(out.str());
}

void Compiler::addError(const MS::Frame& frame, const std::string& message)
{
    std::stringstream out;

    const MS::FrameSet& fs = frame.frameSet();

    out << fs.name() << "." << fs.frames().getName(frame).value()
        << ": " << message;

    _errors.push_back(out.str());
}

void Compiler::addError(const MSC::Animation& ani, const std::string& message)
{
    std::stringstream out;

    const auto& fs = ani.frameSet();

    out << fs.name() << "." << fs.animations().getName(ani).value()
        << ": " << message;

    _errors.push_back(out.str());
}

void Compiler::addWarning(const std::string& message)
{
    _warnings.push_back(message);
}

void Compiler::addWarning(const MS::FrameSet& frameSet, const std::string& message)
{
    std::stringstream out;

    out << frameSet.name() << ": " << message;

    _warnings.push_back(out.str());
}

void Compiler::addWarning(const MS::Frame& frame, const std::string& message)
{
    std::stringstream out;

    const MS::FrameSet& fs = frame.frameSet();

    out << fs.name() << "." << fs.frames().getName(frame).value()
        << ": " << message;

    _warnings.push_back(out.str());
}
