/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilesetcompiler.h"
#include "combinesmalltilesets.h"
#include <climits>
#include <set>

using namespace UnTech;
using namespace UnTech::MetaSprite::Compiler;
namespace MS = UnTech::MetaSprite::MetaSprite;

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

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

    CharAttrPos(unsigned tilesetSplitPoint, unsigned tileOffset)
        : value(tileOffset * 2)
        , charSplitPoint(tilesetSplitPoint * 2)
    {
        if (value >= charSplitPoint) {
            value = CHARATTR_BLOCK_TWO + (tileOffset - tilesetSplitPoint) * 2;
        }
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

// internal representation of a tile
// ids are the tile ids used by the FrameObjects
// if largeTileId == 0xffff then tile is small.
struct TilesetCompiler::Tile16 {
    uint16_t largeTileId = 0xffff;
    std::array<uint16_t, 4> smallTileIds = { { 0xffff, 0xffff, 0xffff, 0xffff } };

    bool isLarge() const { return largeTileId != 0xffff; }
    bool isSmall() const { return largeTileId == 0xffff; }

    bool operator==(const Tile16& o) const
    {
        return largeTileId == o.largeTileId && smallTileIds == o.smallTileIds;
    }
    bool operator!=(const Tile16& o) const
    {
        return !(*this == o);
    }
    bool operator<(const Tile16& o) const
    {
        return std::tie(largeTileId, smallTileIds)
               < std::tie(o.largeTileId, o.smallTileIds);
    }
};

struct TilesetCompiler::FrameTilesetData {
    std::set<const MetaSprite::Frame*> frames;
    std::set<Tile16> tiles;

    FrameTilesetData(const MetaSprite::Frame* frame, std::set<Tile16>&& tiles)
        : frames()
        , tiles(tiles)
    {
        frames.insert(frame);
    }

    FrameTilesetData(const std::vector<FrameListEntry>& frameEntries,
                     std::set<Tile16>&& tiles)
        : frames()
        , tiles(tiles)
    {
        for (const auto& entry : frameEntries) {
            frames.insert(entry.frame);
        }
    }
};

struct TilesetCompiler::DynamicTilesetData {
    std::set<Tile16> staticTiles;
    std::vector<FrameTilesetData> ftVector;
};
}
}
}

TilesetCompiler::TilesetCompiler(ErrorList& errorList,
                                 unsigned tilesetBlockSize)
    : _errorList(errorList)
    , _tileData("TB", "MS_TileBlock", tilesetBlockSize)
    , _tilesetData("TS", "DMA_Tile16Data")
{
}

void TilesetCompiler::writeToIncFile(std::ostream& out) const
{
    _tileData.writeToIncFile(out);
    _tilesetData.writeToIncFile(out);
}

FrameTileset
TilesetCompiler::buildTileset(const MS::FrameSet& frameSet,
                              const TilesetType tilesetType,
                              const std::set<Tile16>& tiles,
                              const unsigned tileOffset)
{
    assert(tiles.size() <= tilesetType.nTiles());

    FrameTileset tileset;

    if (tiles.size() == 0) {
        return tileset;
    }

    RomIncItem tilesetTable;
    tilesetTable.addField(RomIncItem::BYTE, tiles.size());

    auto addTile = [&](const Snes::Tile16px& tile) mutable {
        auto a = _tileData.addLargeTile(tile);
        tilesetTable.addTilePtr(a.addr);

        return a;
    };

    CharAttrPos charAttrPos(tilesetType.tilesetSplitPoint(), tileOffset);

    for (const Tile16& tile16 : tiles) {
        if (tile16.isLarge()) {
            unsigned tId = tile16.largeTileId;
            const Snes::Tile16px& tile = frameSet.largeTileset.tile(tId);

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
        else {
            std::array<Snes::Tile8px, 4> tiles = {};

            for (unsigned i = 0; i < 4; i++) {
                unsigned tId = tile16.smallTileIds[i];
                if (tId < frameSet.smallTileset.size()) {
                    tiles[i] = frameSet.smallTileset.tile(tId);
                }
            }
            RomTileData::Accessor a = addTile(combineSmallTiles(tiles));

            for (unsigned i = 0; i < 4; i++) {
                unsigned tId = tile16.smallTileIds[i];
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

    return tileset;
}

FrameSetTilesets
TilesetCompiler::buildDynamicTileset(const MetaSprite::FrameSet& frameSet,
                                     const TilesetType tilesetType,
                                     const DynamicTilesetData& tileset)
{
    FrameSetTilesets ret;
    ret.tilesetType = tilesetType;

    unsigned staticTileOffset = tilesetType.nTiles() - tileset.staticTiles.size();
    auto staticTileset = buildTileset(frameSet, tilesetType, tileset.staticTiles, staticTileOffset);

    ret.tilesetOffset = staticTileset.tilesetOffset;

    bool error = false;

    for (const auto& ft : tileset.ftVector) {
        if (tileset.staticTiles.size() + ft.tiles.size() <= tilesetType.nTiles()) {

            ret.tilesets.emplace_back(
                buildTileset(frameSet, tilesetType, ft.tiles));

            auto& tileset = ret.tilesets.back();

            tileset.smallTilesetMap.insert(staticTileset.smallTilesetMap.begin(),
                                           staticTileset.smallTilesetMap.end());
            tileset.largeTilesetMap.insert(staticTileset.largeTilesetMap.begin(),
                                           staticTileset.largeTilesetMap.end());

            for (const MS::Frame* frame : ft.frames) {
                ret.frameMap.emplace(frame, tileset);
            }
        }
        else {
            for (const MS::Frame* f : ft.frames) {
                _errorList.addError(frameSet, *f, "Too many tiles in frame");
            }
            error = true;
        }
    }

    if (error) {
        throw std::runtime_error("Unable to build tileset");
    }

    return ret;
}

FrameSetTilesets
TilesetCompiler::buildFixedTileset(const MetaSprite::FrameSet& frameSet,
                                   const TilesetType tilesetType,
                                   const std::set<Tile16>& tiles)
{
    FrameSetTilesets ret;
    ret.tilesetType = tilesetType;

    if (tiles.size() > tilesetType.nTiles()) {
        _errorList.addError(frameSet, "Too many tiles in frameset");
        throw std::runtime_error("Unable to build tileset");
    }

    ret.tilesets.emplace_back(
        buildTileset(frameSet, tilesetType, tiles));
    auto& tileset = ret.tilesets.back();

    ret.tilesetOffset = tileset.tilesetOffset;

    for (const auto& it : frameSet.frames) {
        ret.frameMap.emplace(&it.second, tileset);
    }

    return ret;
}

void TilesetCompiler::addFrameToTileset(std::set<Tile16>& tiles,
                                        const MS::Frame& frame,
                                        const SmallTileMap_t& smallTileMap) const
{
    for (const auto& obj : frame.objects) {
        Tile16 t;
        if (obj.size == ObjectSize::LARGE) {
            t.largeTileId = obj.tileId;
        }
        else {
            t.smallTileIds = smallTileMap.at(obj.tileId);
        }
        tiles.insert(t);
    }
}

std::vector<std::pair<TilesetCompiler::Tile16, unsigned>>
TilesetCompiler::countTileUsage(const std::vector<FrameTilesetData>& ftVector) const
{
    std::vector<std::pair<Tile16, unsigned>> ret;
    ret.reserve(128);

    for (const auto& ft : ftVector) {
        for (const Tile16& ftTile : ft.tiles) {
            auto it = std::find_if(ret.begin(), ret.end(),
                                   [&](const auto& i) { return i.first == ftTile; });
            if (it == ret.end()) {
                ret.emplace_back(ftTile, 1);
            }
            else {
                it->second++;
            }
        }
    }

    return ret;
}

std::set<TilesetCompiler::Tile16>
TilesetCompiler::calculateStaticTiles(const std::vector<FrameTilesetData>& ftVector,
                                      const TilesetType tilesetType) const
{
    auto popularTiles = countTileUsage(ftVector);

    std::stable_sort(popularTiles.begin(), popularTiles.end(),
                     [](auto& l, auto& r) { return l.second > r.second; });

    if (popularTiles.size() > tilesetType.nTiles() - 1) {
        popularTiles.resize(tilesetType.nTiles() - 1);
    }

    while (popularTiles.size() > 0) {
        unsigned maxDynamicTiles = 0;

        for (const auto& ft : ftVector) {
            unsigned nDynamicTiles = ft.tiles.size();

            for (const auto& tc : popularTiles) {
                if (ft.tiles.find(tc.first) != ft.tiles.end()) {
                    nDynamicTiles--;
                }
            }
            if (nDynamicTiles > maxDynamicTiles) {
                maxDynamicTiles = nDynamicTiles;
            }
        }

        if (maxDynamicTiles + popularTiles.size() <= tilesetType.nTiles()) {
            break;
        }
        popularTiles.resize(popularTiles.size() - 1);
    }

    std::set<Tile16> ret;
    for (const auto& tc : popularTiles) {
        ret.emplace(std::move(tc.first));
    }
    return ret;
}

TilesetCompiler::DynamicTilesetData
TilesetCompiler::dynamicTilesetData(const std::vector<FrameListEntry>& frameEntries,
                                    const SmallTileMap_t& smallTileMap,
                                    const TilesetType tilesetType) const
{
    DynamicTilesetData tileset;

    for (const auto& entry : frameEntries) {
        auto& ftVector = tileset.ftVector;

        std::set<Tile16> tiles;
        addFrameToTileset(tiles, *entry.frame, smallTileMap);

        auto it = std::find_if(ftVector.begin(), ftVector.end(),
                               [&](auto& i) { return i.tiles == tiles; });

        if (it == ftVector.end()) {
            ftVector.emplace_back(entry.frame, std::move(tiles));
        }
        else {
            it->frames.insert(entry.frame);
        }
    }

    tileset.staticTiles = calculateStaticTiles(tileset.ftVector, tilesetType);

    // remove staticTiles from ftVector
    for (auto& ft : tileset.ftVector) {
        for (const Tile16& tile : tileset.staticTiles) {
            ft.tiles.erase(tile);
        }
    }

    return tileset;
}

std::set<TilesetCompiler::Tile16>
TilesetCompiler::fixedTilesetData(const std::vector<FrameListEntry>& frames,
                                  const SmallTileMap_t& smallTileMap) const
{
    std::set<Tile16> tiles;

    for (const auto& entry : frames) {
        addFrameToTileset(tiles, *entry.frame, smallTileMap);
    }

    return tiles;
}

SmallTileMap_t
TilesetCompiler::buildSmallTileMap(const std::vector<FrameListEntry>& frameEntries)
{
    TileGraph_t smallTileGraph;

    for (const auto& fle : frameEntries) {
        if (fle.frame != nullptr) {
            for (const auto& obj : fle.frame->objects) {
                if (obj.size == ObjectSize::SMALL) {
                    smallTileGraph[obj.tileId].emplace_back(fle.frame);
                }
            }
        }
    }

    return combineSmallTilesets(smallTileGraph);
}

void TilesetCompiler::validateExportList(const FrameSetExportList& exportList)
{
    const MS::FrameSet& frameSet = exportList.frameSet();

    std::set<unsigned> largeTiles;
    std::set<unsigned> smallTiles;

    for (const auto& fle : exportList.frames()) {
        assert(fle.frame != nullptr);

        for (const auto& obj : fle.frame->objects) {
            if (obj.size == ObjectSize::LARGE) {
                largeTiles.insert(obj.tileId);
            }
            else {
                smallTiles.insert(obj.tileId);
            }
        }
    }

    for (unsigned t : smallTiles) {
        if (t >= frameSet.smallTileset.size()) {
            throw std::runtime_error("Invalid small tileId " + std::to_string(t));
        }
    }
    for (unsigned t : largeTiles) {
        if (t >= frameSet.largeTileset.size()) {
            throw std::runtime_error("Invalid large tileId " + std::to_string(t));
        }
    }

    if (smallTiles.size() == 0 && largeTiles.size() == 0) {
        throw std::runtime_error("No tiles in tileset");
    }
}

FrameSetTilesets
TilesetCompiler::generateTilesets(const FrameSetExportList& exportList)
{
    validateExportList(exportList);
    auto smallTileMap = buildSmallTileMap(exportList.frames());

    const MS::FrameSet& frameSet = exportList.frameSet();
    TilesetType tilesetType = frameSet.tilesetType;

    auto tiles = fixedTilesetData(exportList.frames(), smallTileMap);

    if (tiles.size() <= tilesetType.nTiles() || tilesetType.isFixed()) {
        if (tilesetType.isFixed() == false) {
            _errorList.addWarning(frameSet, "Tileset can be fixed, making it so.");
        }

        auto smallestType = TilesetType::smallestFixedTileset(tiles.size());
        if (tilesetType.nTiles() != smallestType.nTiles()) {
            _errorList.addWarning(frameSet, std::string("TilesetType shrunk to ") + smallestType.string());
        }

        return buildFixedTileset(frameSet, smallestType, tiles);
    }
    else {
        auto tilesetData = dynamicTilesetData(exportList.frames(), smallTileMap, tilesetType);

        return buildDynamicTileset(frameSet, tilesetType, tilesetData);
    }
}
