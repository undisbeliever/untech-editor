/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilesetinserter.h"
#include "tilesetlayout.h"
#include "tilesettype.hpp"
#include "models/common/exceptions.h"
#include "models/common/iterators.h"
#include "models/snes/tilesetinserter.h"

namespace MS = UnTech::MetaSprite::MetaSprite;

namespace UnTech::MetaSprite::Compiler {

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
    const static std::array<uint16_t, 4> SMALL_TILE_OFFSETS;

    CharAttrPos(unsigned tilesetSplitPoint, unsigned tileOffset)
        : value(tileOffset * 2)
        , charSplitPoint(tilesetSplitPoint * 2)
    {
        if (value >= charSplitPoint) {
            value = CHARATTR_BLOCK_TWO + (tileOffset - tilesetSplitPoint) * 2;
        }
    }

    uint16_t largeCharAttr() const { return value | CHARATTR_SIZE_LARGE; }
    uint16_t smallCharAttr(unsigned i) const { return value | SMALL_TILE_OFFSETS.at(i); }

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

const std::array<uint16_t, 4> CharAttrPos::SMALL_TILE_OFFSETS = { 0x0000, 0x0001, 0x0010, 0x0011 };

static FrameTilesetData
insertTiles(const vectorset<Tile16>& tiles, const TilesetType tilesetType,
            const unsigned tileOffset, const FrameTilesetData& staticTileset, const bool dynamicTileset,
            const std::vector<Snes::Tile16px>& largeTileset, const std::vector<Snes::Tile8px>& smallTileset,
            Snes::TilesetInserter16px& tileInserter)
{
    const unsigned tilesetType_nTiles = numberOfTilesetTiles(tilesetType);

    assert(tiles.empty() == false);
    assert(tiles.size() + tileOffset <= tilesetType_nTiles);

    FrameTilesetData tileset;
    tileset.tiles.reserve(tiles.size());
    tileset.smallTilesCharAttr = staticTileset.smallTilesCharAttr;
    tileset.largeTilesCharAttr = staticTileset.largeTilesCharAttr;
    tileset.dynamicTileset = dynamicTileset;

    auto addTile = [&](const Snes::Tile16px& tile) mutable {
        auto a = tileInserter.getOrInsert(tile);
        tileset.tiles.push_back(a.tileId);

        return a;
    };

    CharAttrPos charAttrPos(tilesetSplitPoint(tilesetType), tileOffset);

    for (const Tile16& tile16 : tiles) {
        if (tile16.isLarge()) {
            unsigned tId = tile16.largeTileId;
            const Snes::Tile16px& tile = largeTileset.at(tId);

            const auto a = addTile(tile);

            uint16_t charAttr = charAttrPos.largeCharAttr();
            if (a.hFlip) {
                charAttr |= CharAttrPos::CHARATTR_HFLIP;
            }
            if (a.vFlip) {
                charAttr |= CharAttrPos::CHARATTR_VFLIP;
            }
            tileset.largeTilesCharAttr.at(tId) = charAttr;

            charAttrPos.inc();
        }
        else {
            if (tile16.smallTileIds == INVALID_SMALL_TILES_ARRAY) {
                throw logic_error(u8"Invalid smallTileIds");
            }

            std::array<Snes::Tile8px, 4> smallTiles = {};

            for (const auto i : range(4)) {
                unsigned tId = tile16.smallTileIds[i];
                if (tId < smallTileset.size()) {
                    smallTiles[i] = smallTileset.at(tId);
                }
            }
            const auto a = addTile(combineSmallTiles(smallTiles));

            for (const auto i : range(4)) {
                auto tId = tile16.smallTileIds[i];

                if (tId >= INVALID_SMALL_TILE) {
                    continue;
                }

                uint16_t charAttr = charAttrPos.smallCharAttr(i);

                if (a.hFlip) {
                    charAttr |= CharAttrPos::CHARATTR_HFLIP;
                }
                if (a.vFlip) {
                    charAttr |= CharAttrPos::CHARATTR_VFLIP;
                }
                tileset.smallTilesCharAttr.at(tId) = charAttr;
            }

            charAttrPos.inc();
        }
    }

    assert(!tileset.tiles.empty());

    return tileset;
}

static FrameTilesetData
insertStaticTileset(const vectorset<Tile16>& tiles, const TilesetType tilesetType,
                    const unsigned tileOffset,
                    const std::vector<Snes::Tile16px>& largeTileset, const std::vector<Snes::Tile8px>& smallTileset,
                    Snes::TilesetInserter16px& tileInserter)
{
    constexpr uint16_t nullTile = FrameTilesetData::NULL_CHAR_ATTR;

    FrameTilesetData blankTileset;
    blankTileset.largeTilesCharAttr.resize(largeTileset.size(), nullTile);
    blankTileset.smallTilesCharAttr.resize(smallTileset.size(), nullTile);

    if (tiles.empty() == false) {
        return insertTiles(tiles, tilesetType, tileOffset, blankTileset, false,
                           largeTileset, smallTileset, tileInserter);
    }
    else {
        return blankTileset;
    }
}

static FrameTilesetData
insertDynamicTileset(const std::vector<Tile16>& tiles, const TilesetType tilesetType,
                     const FrameTilesetData& staticTileset,
                     const std::vector<Snes::Tile16px>& largeTileset, const std::vector<Snes::Tile8px>& smallTileset,
                     Snes::TilesetInserter16px& tileInserter)
{
    return insertTiles(tiles, tilesetType, 0, staticTileset, true,
                       largeTileset, smallTileset, tileInserter);
}

TilesetData processTileset(const MetaSprite::FrameSet& frameSet, const TilesetLayout& tilesetLayout)
{
    TilesetData ret;

    Snes::TilesetInserter16px tileInserter(ret.tiles);

    ret.tilesetTypeByte = tilesetTypeRomValue(tilesetLayout.tilesetType);
    ret.frameTilesets = tilesetLayout.frameTilesets;

    const unsigned tilesetType_nTiles = numberOfTilesetTiles(tilesetLayout.tilesetType);

    unsigned staticTilesOffset = 0;
    if (not isFixedTilesetType(tilesetLayout.tilesetType)) {
        assert(tilesetType_nTiles >= tilesetLayout.staticTiles.size());
        staticTilesOffset = tilesetType_nTiles - tilesetLayout.staticTiles.size();
    }

    ret.staticTileset = insertStaticTileset(tilesetLayout.staticTiles, tilesetLayout.tilesetType, staticTilesOffset,
                                            frameSet.largeTileset, frameSet.smallTileset,
                                            tileInserter);

    for (const auto& dynTiles : tilesetLayout.dynamicTiles) {
        ret.dynamicTilesets.push_back(
            insertDynamicTileset(dynTiles, tilesetLayout.tilesetType, ret.staticTileset,
                                 frameSet.largeTileset, frameSet.smallTileset,
                                 tileInserter));
    }

    return ret;
}

}
