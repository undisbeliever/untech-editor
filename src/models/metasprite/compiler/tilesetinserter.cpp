/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
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

class RomDmaTile16Entry {
    constexpr static unsigned MAX_TILES_PER_TILESET = 16;

    std::vector<uint8_t> _data;
    unsigned _tileCount = 0;

public:
    RomDmaTile16Entry()
        : _data(1)
        , _tileCount()
    {
    }

    void addTile(const uint16_t a)
    {
        _data.push_back(a);
        _data.push_back(a >> 8);

        _tileCount++;
    }

    const std::vector<uint8_t>& commitData()
    {
        assert(_tileCount <= MAX_TILES_PER_TILESET);

        _data.at(0) = _tileCount;
        return _data;
    }
};

const uint16_t CharAttrPos::SMALL_TILE_OFFSETS[4] = { 0x0000, 0x0001, 0x0010, 0x0011 };

static FrameTilesetData
insertTiles(const vectorset<Tile16>& tiles, const TilesetType tilesetType,
            const unsigned tileOffset, const FrameTilesetData& staticTileset, const bool dynamicTileset,
            const Snes::TilesetTile16& largeTileset, const Snes::Tileset8px& smallTileset,
            CompiledRomData& out)
{
    assert(tiles.empty() == false);
    assert(tiles.size() + tileOffset <= tilesetType.nTiles());

    FrameTilesetData tileset = staticTileset;
    tileset.dynamicTileset = dynamicTileset;

    RomDmaTile16Entry tilesetEntry;

    auto addTile = [&](const Snes::Tile16px& tile) mutable {
        auto a = out.tileData.addLargeTile(tile);
        tilesetEntry.addTile(a.tile16Addr);

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

    tileset.tilesetIndex = out.dmaTile16Data.addData_IndexPlusOne(tilesetEntry.commitData());

    return tileset;
}

static FrameTilesetData
insertStaticTileset(const vectorset<Tile16>& tiles, const TilesetType tilesetType,
                    const unsigned tileOffset,
                    const Snes::TilesetTile16& largeTileset, const Snes::Tileset8px& smallTileset,
                    CompiledRomData& out)
{
    constexpr uint16_t nullTile = FrameTilesetData::NULL_CHAR_ATTR;

    FrameTilesetData blankTileset;
    blankTileset.largeTilesCharAttr.resize(largeTileset.size(), nullTile);
    blankTileset.smallTilesCharAttr.resize(smallTileset.size(), nullTile);

    if (tiles.empty() == false) {
        return insertTiles(tiles, tilesetType, tileOffset, blankTileset, false,
                           largeTileset, smallTileset, out);
    }
    else {
        return blankTileset;
    }
}

static FrameTilesetData
insertDynamicTileset(const std::vector<Tile16>& tiles, const TilesetType tilesetType,
                     const FrameTilesetData& staticTileset,
                     const Snes::TilesetTile16& largeTileset, const Snes::Tileset8px& smallTileset,
                     CompiledRomData& out)
{
    return insertTiles(tiles, tilesetType, 0, staticTileset, true,
                       largeTileset, smallTileset, out);
}

TilesetData insertFrameSetTiles(const MS::FrameSet& frameSet, const TilesetLayout& tilesetLayout,
                                CompiledRomData& out)
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
                                            out);

    for (const auto& dynTiles : tilesetLayout.dynamicTiles) {
        ret.dynamicTilesets.push_back(
            insertDynamicTileset(dynTiles, tilesetLayout.tilesetType, ret.staticTileset,
                                 frameSet.largeTileset, frameSet.smallTileset,
                                 out));
    }

    return ret;
}

}
}
}
