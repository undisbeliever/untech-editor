/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../common/image.h"
#include "models/common/attributes.h"
#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace UnTech::Snes {

template <std::size_t TS>
class Tile {
public:
    constexpr static unsigned TILE_SIZE = TS;
    constexpr static unsigned TILE_ARRAY_SIZE = TILE_SIZE * TILE_SIZE;

    using tileArray_t = std::array<uint8_t, TILE_ARRAY_SIZE>;

protected:
    std::array<uint8_t, TILE_ARRAY_SIZE> _data = {};

public:
    Tile() = default;

public:
    inline tileArray_t& data() { return _data; }
    inline const tileArray_t& data() const { return _data; }
    inline void setData(const tileArray_t& data) { _data = data; }

    [[nodiscard]] inline std::span<const uint8_t> sliver(const unsigned y) const
    {
        if (y >= TILE_SIZE) {
            throw invalid_argument(u8"Tile::sliver: y too large");
        }
        return std::span(_data.data() + y * TILE_SIZE, TILE_SIZE);
    }

    Tile hFlip() const;
    Tile vFlip() const;
    Tile hvFlip() const;

    Tile flip(bool h_flip, bool v_flip) const;

    [[nodiscard]] uint8_t pixel(unsigned x, unsigned y) const
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

    bool operator==(const Tile&) const = default;
};

using Tile8px = Tile<8>;
using Tile16px = Tile<16>;

std::array<Tile8px, 4> splitLargeTile(const Tile16px& largeTile);
Tile16px combineSmallTiles(const std::array<Tile8px, 4>& tiles);

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
