#ifndef _UNTECH_MODELS_SNES_TILESET_H_
#define _UNTECH_MODELS_SNES_TILESET_H_

#include "palette.h"
#include "../common/image.h"
#include <cstdint>
#include <vector>
#include <array>

namespace UnTech {
namespace Snes {

template <size_t BIT_DEPTH, size_t TILE_SIZE>
class Tileset {
    static_assert(BIT_DEPTH <= 8, "BIT_DEPTH is too high");
    static_assert((BIT_DEPTH & 1) == 0, "BIT_DEPTH must be a multiple of 2");

public:
    const static unsigned PIXEL_MASK = (1 << BIT_DEPTH) - 1;
    const static unsigned SNES_DATA_SIZE = TILE_SIZE * TILE_SIZE * BIT_DEPTH / 8;

    typedef std::array<uint8_t, TILE_SIZE * TILE_SIZE> tileData_t;

public:
    void drawTile(Image& imgage, const Palette<BIT_DEPTH>& palette,
                  unsigned xOffset, unsigned yOffset,
                  unsigned tileId, bool hFlip = false, bool vFlip = false) const;

    void addTile() { _tiles.emplace_back(); }

    size_t size() const { return _tiles.size(); }

    tileData_t& tile(size_t n) { return _tiles.at(n); }
    const tileData_t& tile(size_t n) const { return _tiles.at(n); }

    uint8_t tilePixel(unsigned tileId, unsigned x, unsigned y) const
    {
        return _tiles.at(tileId).at(y * TILE_SIZE + x);
    }

    void setTilePixel(unsigned tileId, unsigned x, unsigned y, uint8_t value)
    {
        _tiles.at(tileId).at(y * TILE_SIZE + x) = value & PIXEL_MASK;
    }

protected:
    std::vector<tileData_t> _tiles;
};

template <size_t BIT_DEPTH>
class Tileset8px : public Tileset<BIT_DEPTH, 8> {
public:
    // ::SHOULDDO a drawTile16 method::

    void readSnesData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> snesData() const;
};

/**
 * NOTE: Tileset<N, 16> tiles are stored/loaded sequentially.
 *       This is the way 16px tiles are stored in UnTech engine ROM,
 *       not in the SNES' VRAM.
 *
 */
template <size_t BIT_DEPTH>
class Tileset16px : public Tileset<BIT_DEPTH, 16> {
public:
    void readSnesData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> snesData() const;
};

typedef Tileset8px<2> Tileset2bpp8px;
typedef Tileset8px<4> Tileset4bpp8px;
typedef Tileset8px<8> Tileset8bpp8px;

typedef Tileset16px<4> Tileset4bpp16px;
}
}
#endif
