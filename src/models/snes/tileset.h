/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "palette.h"
#include "tile.h"
#include "../common/image.h"
#include <array>
#include <cstdint>
#include <vector>

namespace UnTech {
namespace Snes {

template <size_t TS>
class BaseTileset {
public:
    using TileT = Tile<TS>;
    using iterator = typename std::vector<TileT>::iterator;

public:
    constexpr static unsigned TILE_SIZE = TS;

    BaseTileset() = default;

    void addTile() { _tiles.emplace_back(); }
    void addTile(const TileT& tile) { _tiles.emplace_back(tile); }
    void addTile(unsigned tileId, const TileT& tile)
    {
        if (tileId > _tiles.size()) {
            tileId = _tiles.size();
        }
        _tiles.insert(_tiles.begin() + tileId, tile);
    }

    TileT& tile(size_t n) { return _tiles.at(n); }
    const TileT& tile(size_t n) const { return _tiles.at(n); }

    // expose vector
    size_t size() const { return _tiles.size(); }
    bool empty() const { return _tiles.empty(); }
    auto begin() { return _tiles.begin(); }
    auto begin() const { return _tiles.begin(); }
    auto end() { return _tiles.begin(); }
    auto end() const { return _tiles.begin(); }

    // Remove tile
    void removeLastTile()
    {
        if (size() > 0) {
            _tiles.pop_back();
        }
    }
    void removeTile(unsigned tileId)
    {
        if (size() > 0 && tileId < size()) {
            _tiles.erase(_tiles.begin() + tileId);
        }
    }

    // accessor methods
    TileT& at(unsigned tileId) { return _tiles.at(tileId); }
    const TileT& at(unsigned tileId) const { return _tiles.at(tileId); }

    void insert(iterator it, const TileT& tile) { _tiles.insert(it, tile); }
    void erase(iterator it) { _tiles.erase(it); }

protected:
    std::vector<TileT> _tiles;
};

class Tileset8px : public BaseTileset<8> {
public:
    enum class BitDepth {
        BD_1BPP = 1,
        BD_2BPP = 2,
        BD_3BPP = 3,
        BD_4BPP = 4,
        BD_8BPP = 8
    };

    static BitDepth depthFromInt(int bd);

public:
    Tileset8px(BitDepth bitDepth)
        : BaseTileset()
        , _bitDepth(bitDepth)
    {
    }

    Tileset8px(int bitDepth)
        : BaseTileset()
        , _bitDepth(depthFromInt(bitDepth))
    {
    }

    void setBitDepth(BitDepth bitDepth) { _bitDepth = bitDepth; }

    BitDepth bitDepth() const { return _bitDepth; }
    int bitDepthInt() const { return int(_bitDepth); }

    uint8_t pixelMask() const { return (1 << unsigned(_bitDepth)) - 1; }
    unsigned colorsPerTile() const { return 1 << unsigned(_bitDepth); }
    unsigned snesTileSize() const { return TILE_SIZE * TILE_SIZE * unsigned(_bitDepth) / 8; }

    size_t snesDataSize() const { return snesTileSize() * size(); }

    // fails silently
    template <size_t BD>
    void drawTile(Image& image, const Palette<BD>& palette,
                  unsigned xOffset, unsigned yOffset,
                  unsigned tileId, bool hFlip = false, bool vFlip = false) const;

    std::vector<uint8_t> snesData() const;
    void readSnesData(const std::vector<uint8_t>& data);

    bool operator==(const Tileset8px& o) const
    {
        return _tiles == o._tiles
               && _bitDepth == o._bitDepth;
    }
    bool operator!=(const Tileset8px& o) const { return !(*this == o); }

private:
    BitDepth _bitDepth;
};

/**
 * TilesetTile16 tiles are stored/loaded in sequential order.
 * This matches the Tile16 format used by the UnTech engine to store
 * the MetaSprite tiles in ROM.
 */
class TilesetTile16 : public BaseTileset<16> {
public:
    constexpr static unsigned BIT_DEPTH = 4;
    constexpr static unsigned SNES_SMALL_TILE_SIZE = 8 * BIT_DEPTH;
    constexpr static unsigned SNES_TILE_SIZE = SNES_SMALL_TILE_SIZE * 4;

    TilesetTile16() = default;

    // fails silently
    void drawTile(Image& image, const Palette<4>& palette,
                  unsigned xOffset, unsigned yOffset,
                  unsigned tileId, bool hFlip = false, bool vFlip = false) const;

    std::vector<uint8_t> snesData() const;
    void readSnesData(const std::vector<uint8_t>& data);

    bool operator==(const TilesetTile16& o) const { return _tiles == o._tiles; }
    bool operator!=(const TilesetTile16& o) const { return _tiles != o._tiles; }
};
}
}
