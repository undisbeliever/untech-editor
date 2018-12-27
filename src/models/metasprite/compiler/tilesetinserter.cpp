/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilesetinserter.h"
#include "tilesetlayout.h"

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

static FrameTilesetData
insertTiles(const std::vector<Tile16>& tiles, const TilesetType tilesetType,
            const unsigned tileOffset, const FrameTilesetData& staticTileset, const bool dynamicTileset,
            const Snes::TilesetTile16& largeTileset, const Snes::Tileset8px& smallTileset,
            RomTileData& tileData, RomIncData& tilesetData)
{
    assert(tiles.empty() == false);
    assert(tiles.size() + tileOffset <= tilesetType.nTiles());

    FrameTilesetData tileset = staticTileset;
    tileset.dynamicTileset = dynamicTileset;

    RomIncItem tilesetTable;
    tilesetTable.addField(RomIncItem::BYTE, tiles.size());

    auto addTile = [&](const Snes::Tile16px& tile) mutable {
        auto a = tileData.addLargeTile(tile);
        tilesetTable.addTilePtr(a.addr);

        return a;
    };

    CharAttrPos charAttrPos(tilesetType.tilesetSplitPoint(), tileOffset);

    for (const Tile16& tile16 : tiles) {
        if (tile16.isLarge()) {
            unsigned tId = tile16.largeTileId;
            const Snes::Tile16px& tile = largeTileset.tile(tId);

            RomTileData::Accessor a = addTile(tile);

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
                throw std::logic_error("Invalid smallTileIds");
            }

            std::array<Snes::Tile8px, 4> smallTiles = {};

            for (unsigned i = 0; i < 4; i++) {
                unsigned tId = tile16.smallTileIds[i];
                if (tId < smallTileset.size()) {
                    smallTiles[i] = smallTileset.tile(tId);
                }
            }
            RomTileData::Accessor a = addTile(combineSmallTiles(smallTiles));

            for (unsigned i = 0; i < 4; i++) {
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

    // Store the DMA data and tileset
    tileset.romPtr = tilesetData.addData(tilesetTable);

    return tileset;
}

static FrameTilesetData
insertStaticTileset(const std::vector<Tile16>& tiles, const TilesetType tilesetType,
                    const unsigned tileOffset,
                    const Snes::TilesetTile16& largeTileset, const Snes::Tileset8px& smallTileset,
                    RomTileData& tileData, RomIncData& tilesetData)
{
    constexpr uint16_t nullTile = FrameTilesetData::NULL_CHAR_ATTR;

    FrameTilesetData blankTileset;
    blankTileset.largeTilesCharAttr.resize(largeTileset.size(), nullTile);
    blankTileset.smallTilesCharAttr.resize(smallTileset.size(), nullTile);

    if (tiles.empty() == false) {
        return insertTiles(tiles, tilesetType, tileOffset, blankTileset, false,
                           largeTileset, smallTileset, tileData, tilesetData);
    }
    else {
        return blankTileset;
    }
}

static FrameTilesetData
insertDynamicTileset(const std::vector<Tile16>& tiles, const TilesetType tilesetType,
                     const FrameTilesetData& staticTileset,
                     const Snes::TilesetTile16& largeTileset, const Snes::Tileset8px& smallTileset,
                     RomTileData& tileData, RomIncData& tilesetData)
{
    return insertTiles(tiles, tilesetType, 0, staticTileset, true,
                       largeTileset, smallTileset, tileData, tilesetData);
}

TilesetData insertFrameSetTiles(const MS::FrameSet& frameSet, const TilesetLayout& tilesetLayout,
                                RomTileData& tileData, RomIncData& tilesetData)
{
    TilesetData ret;

    ret.tilesetType = tilesetLayout.tilesetType;
    ret.frameTilesets = tilesetLayout.frameTilesets;

    unsigned staticTilesOffset = 0;
    if (ret.tilesetType.isFixed() == false) {
        assert(ret.tilesetType.nTiles() >= tilesetLayout.staticTiles.size());
        staticTilesOffset = ret.tilesetType.nTiles() - tilesetLayout.staticTiles.size();
    }

    ret.staticTileset = insertStaticTileset(tilesetLayout.staticTiles, tilesetLayout.tilesetType, staticTilesOffset,
                                            frameSet.largeTileset, frameSet.smallTileset,
                                            tileData, tilesetData);

    for (const auto& dynTiles : tilesetLayout.dynamicTiles) {
        ret.dynamicTilesets.push_back(
            insertDynamicTileset(dynTiles, tilesetLayout.tilesetType, ret.staticTileset,
                                 frameSet.largeTileset, frameSet.smallTileset,
                                 tileData, tilesetData));
    }

    return ret;
}

}
}
}
