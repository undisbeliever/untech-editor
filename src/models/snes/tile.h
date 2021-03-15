/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../common/image.h"
#include "models/common/attributes.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

namespace UnTech {
namespace Snes {

template <size_t TS>
class Tile {
public:
    constexpr static unsigned TILE_SIZE = TS;
    constexpr static unsigned TILE_ARRAY_SIZE = TILE_SIZE * TILE_SIZE;

    typedef std::array<uint8_t, TILE_ARRAY_SIZE> tileArray_t;

public:
    Tile() = default;

public:
    inline uint8_t* rawData() { return _data.data(); }
    inline const uint8_t* rawData() const { return _data.data(); }

    inline const auto& data() const { return _data; }
    inline void setData(std::array<uint8_t, TILE_ARRAY_SIZE> data) { _data = data; }

    Tile flip(bool hFlip, bool vFlip) const;
    Tile hFlip() const;
    Tile vFlip() const;
    Tile hvFlip() const;

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
            _data[y * TILE_SIZE + x] = value;
        }
    }

    bool operator==(const Tile& other) const { return this->_data == other._data; }
    bool operator!=(const Tile& other) const { return this->_data != other._data; }

protected:
    std::array<uint8_t, TILE_ARRAY_SIZE> _data = {};
};

using Tile8px = Tile<8>;
using Tile16px = Tile<16>;

std::array<Tile8px, 4> splitLargeTile(const Tile16px& largeTile);
Tile16px combineSmallTiles(const std::array<Tile8px, 4>& tiles);
}
}

namespace std {
template <size_t TS>
struct hash<::UnTech::Snes::Tile<TS>> {
    size_t operator()(const ::UnTech::Snes::Tile<TS>& tile) const
        __attribute__(IGNORE_UNSIGNED_OVERFLOW_ATTR)
    {
        size_t seed = 0;
        for (const auto d : tile.data()) {
            // Numbers from boost
            seed ^= d + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        return seed;
    }
};
template <size_t TS>
struct hash<std::vector<::UnTech::Snes::Tile<TS>>> {
    size_t operator()(const std::vector<::UnTech::Snes::Tile<TS>>& tiles) const
        __attribute__(IGNORE_UNSIGNED_OVERFLOW_ATTR)
    {
        size_t seed = 0;
        for (const auto& tile : tiles) {
            for (const auto d : tile.data()) { // Numbers from boost
                seed ^= d + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
        }

        return seed;
    }
};
}
