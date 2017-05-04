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

public:
    BaseTileset() = default;

    void addTile() { _tiles.emplace_back(); }
    void addTile(const TileT& tile) { _tiles.emplace_back(tile); }

    TileT& tile(size_t n) { return _tiles.at(n); }
    const TileT& tile(size_t n) const { return _tiles.at(n); }

    // expose vector
    size_t size() const { return _tiles.size(); }
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

protected:
    std::vector<TileT> _tiles;
};

template <size_t BD>
class Tileset8px : public BaseTileset<8> {
public:
    constexpr static unsigned BIT_DEPTH = BD;
    constexpr static unsigned SNES_TILE_SIZE = 8 * BIT_DEPTH;

    Tileset8px() = default;

    // fails silently
    void drawTile(Image& image, const Palette<BD>& palette,
                  unsigned xOffset, unsigned yOffset,
                  unsigned tileId, bool hFlip = false, bool vFlip = false) const;

    std::vector<uint8_t> snesData() const;
    void readSnesData(const std::vector<uint8_t>& data);
};

typedef Tileset8px<1> Tileset1bpp8px;
typedef Tileset8px<2> Tileset2bpp8px;
typedef Tileset8px<3> Tileset3bpp8px;
typedef Tileset8px<4> Tileset4bpp8px;
typedef Tileset8px<8> Tileset8bpp8px;

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
};
}
}
