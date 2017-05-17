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

void TilesetCompiler::buildTileset(FrameTileset& tileset,
                                   const MS::FrameSet& frameSet,
                                   const TilesetType& tilesetType,
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
        auto addTile = [&](const Snes::Tile16px& tile) mutable {
            auto a = _tileData.addLargeTile(tile);
            tilesetTable.addTilePtr(a.addr);

            return a;
        };

        // Process Large Tiles
        CharAttrPos charAttrPos(tilesetType.tilesetSplitPoint());

        for (unsigned tId : largeTiles) {
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

        // Process Small Tiles
        for (auto tileIds : smallTiles) {
            std::array<Snes::Tile8px, 4> tiles = {};

            for (unsigned i = 0; i < 4; i++) {
                unsigned tId = tileIds[i];
                if (tId < frameSet.smallTileset.size()) {
                    tiles[i] = frameSet.smallTileset.tile(tId);
                }
            }
            RomTileData::Accessor a = addTile(combineSmallTiles(tiles));

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

void TilesetCompiler::buildTileset(FrameTileset& tileset,
                                   const MS::FrameSet& frameSet,
                                   const TilesetType& tilesetType,
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

inline FrameTilesetList
TilesetCompiler::generateDynamicTilesets(const MetaSprite::FrameSet& frameSet,
                                         const TilesetType& tilesetType,
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

        bool error = false;

        for (const auto& ft : ftData) {
            ret.tilesets.emplace_back();
            auto& tileset = ret.tilesets.back();

            if (ft.nTiles() <= tilesetType.nTiles()) {
                buildTileset(tileset, frameSet, tilesetType, ft.largeTiles, ft.smallTiles);
            }
            else if (ft.nTilesUnoptimized() <= tilesetType.nTiles()) {
                _errorList.addWarning(frameSet, "Could not optimally combine tileset");
                buildTileset(tileset, frameSet, tilesetType, ft.largeTiles, ft.originalSmallTiles);
            }
            else {
                for (const auto* f : ft.frames) {
                    _errorList.addError(frameSet, *f, "Too many tiles in frame");
                }
                error = true;
            }

            for (const MS::Frame* frame : ft.frames) {
                ret.frameMap.emplace(frame, tileset);
            }
        }

        if (error) {
            throw std::runtime_error("Unable to build tileset");
        }

        return ret;
    }
}

inline FrameTilesetList
TilesetCompiler::generateFixedTileset(const MetaSprite::FrameSet& frameSet,
                                      const TilesetType& tilesetType,
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

        for (const auto& fsIt : frameSet.frames) {
            ret.frameMap.emplace(&fsIt.second, tileset);
        }

        return ret;
    }
}

FrameTilesetList
TilesetCompiler::generateTilesetList(const FrameSetExportList& exportList)
{
    const MS::FrameSet& frameSet = exportList.frameSet();

    TileGraph_t largeTileGraph;
    TileGraph_t smallTileGraph;

    // Build a graph of the tiles and the frames that use them.
    for (const auto& fle : exportList.frames()) {
        if (fle.frame != nullptr) {
            for (const auto& obj : fle.frame->objects) {

                if (obj.size == ObjectSize::LARGE) {
                    largeTileGraph[obj.tileId].emplace_back(fle.frame);
                }
                else {
                    smallTileGraph[obj.tileId].emplace_back(fle.frame);
                }
            }
        }
    }

    // verify tileIds are valid
    {
        for (const auto& it : smallTileGraph) {
            unsigned t = it.first;
            if (t >= frameSet.smallTileset.size()) {
                throw std::runtime_error("Invalid small tileId " + std::to_string(t));
            }
        }
        for (const auto& it : largeTileGraph) {
            unsigned t = it.first;
            if (t >= frameSet.largeTileset.size()) {
                throw std::runtime_error("Invalid large tileId " + std::to_string(t));
            }
        }
    }

    unsigned tilesetSize = largeTileGraph.size() + (smallTileGraph.size() + 3) / 4;
    auto tilesetType = frameSet.tilesetType;

    if (tilesetSize == 0) {
        throw std::runtime_error("No tiles in tileset");
    }

    if (tilesetSize <= tilesetType.nTiles() || tilesetType.isFixed()) {
        // All of the tiles used in the frameset
        if (tilesetType.isFixed() == false) {
            _errorList.addWarning(frameSet, "Tileset can be fixed, making it so.");
        }

        // try to resize tileset if necessary
        auto smallestType = TilesetType::smallestFixedTileset(tilesetSize);

        if (tilesetType.nTiles() != smallestType.nTiles()) {
            _errorList.addWarning(frameSet, std::string("TilesetType shrunk to ") + smallestType.string());
        }

        return generateFixedTileset(frameSet, smallestType, largeTileGraph, smallTileGraph);
    }
    else {
        return generateDynamicTilesets(frameSet, tilesetType, largeTileGraph, smallTileGraph);
    }
}
