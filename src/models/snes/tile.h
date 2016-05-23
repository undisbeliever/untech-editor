#pragma once

#include "palette.h"
#include "../common/image.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

namespace UnTech {
namespace Snes {

template <size_t BD, size_t TS>
struct TileTraits {
};

template <size_t BD, size_t TS>
class Tile {
    static_assert(BD <= 8, "BD is too high");
    static_assert((BD & 1) == 0, "BD must be a multiple of 2");

public:
    constexpr static unsigned TILE_SIZE = TS;
    constexpr static unsigned BIT_DEPTH = BD;

    constexpr static unsigned PIXEL_MASK = (1 << BD) - 1;
    constexpr static unsigned TILE_ARRAY_SIZE = TILE_SIZE * TILE_SIZE;
    constexpr static unsigned SNES_DATA_SIZE = TILE_ARRAY_SIZE * BD / 8;

    typedef std::array<uint8_t, TILE_ARRAY_SIZE> tileArray_t;
    typedef Palette<BD> palette_t;

    typedef typename TileTraits<BD, TS>::Tile_t Tile_t;

public:
    Tile() = default;
    Tile(const Tile&) = default;

public:
    // fails silently
    void draw(Image& image, const Palette<BD>& palette,
              unsigned xOffset, unsigned yOffset,
              bool hFlip = false, bool vFlip = false) const;

    inline uint8_t* rawData() { return _data.data(); }
    inline const uint8_t* rawData() const { return _data.data(); }

    Tile_t flip(bool hFlip, bool vFlip) const;
    Tile_t hFlip() const;
    Tile_t vFlip() const;
    Tile_t hvFlip() const;

    uint8_t pixel(unsigned x, unsigned y) const
    {
        if (x < TILE_SIZE && y < TILE_SIZE) {
            return _data[y * TILE_SIZE + x];
        }
        else {
            return 0;
        }
    }

    void setPixel(unsigned x, unsigned y, uint8_t value)
    {
        if (x < TILE_SIZE && y < TILE_SIZE) {
            _data[y * TILE_SIZE + x] = value & PIXEL_MASK;
        }
    }

    bool operator==(const Tile& other) const { return this->_data == other._data; }
    bool operator!=(const Tile& other) const { return this->_data != other._data; }

protected:
    std::array<uint8_t, TILE_ARRAY_SIZE> _data;
};

template <size_t BD>
class Tile8px : public Tile<BD, 8> {
public:
    constexpr static unsigned TILE_SIZE = 8;
    constexpr static unsigned BIT_DEPTH = BD;

    constexpr static unsigned PIXEL_MASK = (1 << BD) - 1;
    constexpr static unsigned TILE_ARRAY_SIZE = TILE_SIZE * TILE_SIZE;
    constexpr static unsigned SNES_DATA_SIZE = TILE_ARRAY_SIZE * BD / 8;

public:
    Tile8px() = default;
    Tile8px(const Tile8px&) = default;
    Tile8px(const Tile<BD, 8>&);
    Tile8px(const uint8_t data[SNES_DATA_SIZE]) { readSnesData(data); }

    void readSnesData(const uint8_t data[SNES_DATA_SIZE]);
    void writeSnesData(uint8_t out[SNES_DATA_SIZE]) const;
};

/**
 * NOTE: Tile<N, 16> tiles are stored/loaded sequentially.
 *       This is the way 16px tiles are stored in UnTech engine ROM,
 *       not in the SNES' VRAM.
 *
 */
template <size_t BD>
class Tile16px : public Tile<BD, 16> {
public:
    constexpr static unsigned TILE_SIZE = 16;
    constexpr static unsigned BIT_DEPTH = BD;

    constexpr static unsigned PIXEL_MASK = (1 << BD) - 1;
    constexpr static unsigned TILE_ARRAY_SIZE = TILE_SIZE * TILE_SIZE;
    constexpr static unsigned SNES_DATA_SIZE = TILE_ARRAY_SIZE * BD / 8;

public:
    Tile16px() = default;
    Tile16px(const Tile16px&) = default;
    Tile16px(const Tile<BD, 16>&);
    Tile16px(const uint8_t data[SNES_DATA_SIZE]) { readSnesData(data); }
    Tile16px(const std::array<Tile8px<BD>, 4>& tiles) { combineIntoLarge(tiles); }

    void readSnesData(const uint8_t data[SNES_DATA_SIZE]);
    void writeSnesData(uint8_t out[SNES_DATA_SIZE]) const;

    std::array<Tile8px<BD>, 4> splitIntoSmall() const;
    void combineIntoLarge(const std::array<Tile8px<BD>, 4>& tiles);
};

typedef Tile8px<2> Tile2bpp8px;
typedef Tile8px<4> Tile4bpp8px;
typedef Tile8px<8> Tile8bpp8px;

typedef Tile16px<4> Tile4bpp16px;

template <>
struct TileTraits<2, 8> {
    typedef Tile2bpp8px Tile_t;
};
template <>
struct TileTraits<4, 8> {
    typedef Tile4bpp8px Tile_t;
};
template <>
struct TileTraits<8, 8> {
    typedef Tile8bpp8px Tile_t;
};
template <>
struct TileTraits<4, 16> {
    typedef Tile4bpp16px Tile_t;
};
}
}
