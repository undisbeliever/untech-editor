/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tile.h"
#include "models/common/iterators.h"
#include <cstring>

namespace UnTech::Snes {

template <size_t TS>
Tile<TS> Tile<TS>::flip(bool hFlip, bool vFlip) const
{
    Tile<TS> ret;
    const uint8_t* pixelData = this->rawData();

    for (const auto y : range(TS)) {
        unsigned fy = (vFlip == false) ? y : TS - 1 - y;
        uint8_t* retRow = ret.rawData() + fy * TS;

        for (const auto x : range(TS)) {
            unsigned fx = (hFlip == false) ? x : TS - 1 - x;
            retRow[fx] = *pixelData++;
        }
    }

    return ret;
}

template <size_t TS>
Tile<TS> Tile<TS>::hFlip() const
{
    return flip(true, false);
}

template <size_t TS>
Tile<TS> Tile<TS>::vFlip() const
{
    return flip(false, true);
}

template <size_t TS>
Tile<TS> Tile<TS>::hvFlip() const
{
    return flip(true, true);
}

template class Snes::Tile<8>;
template class Snes::Tile<16>;

std::array<Tile8px, 4> splitLargeTile(const Tile16px& largeTile)
{
    std::array<Tile8px, 4> ret;
    const uint8_t* tile16 = largeTile.rawData();

    auto transform = [&tile16](Tile8px& tile8,
                               unsigned xPos, unsigned yPos) {
        const uint8_t* t16 = tile16 + (yPos * 16 + xPos);
        uint8_t* t8 = tile8.rawData();

        for ([[maybe_unused]] const auto y : range(8)) {
            memcpy(t8, t16, 8);
            t16 += 16;
            t8 += 8;
        }
    };

    transform(ret[0], 0, 0);
    transform(ret[1], 8, 0);
    transform(ret[2], 0, 8);
    transform(ret[3], 8, 8);

    return ret;
}

Tile16px combineSmallTiles(const std::array<Tile8px, 4>& tiles)
{
    Tile<16> ret;
    uint8_t* tile16 = ret.rawData();

    auto transform = [&tile16](const Tile8px& tile8,
                               unsigned xPos, unsigned yPos) {
        const uint8_t* t8 = tile8.rawData();
        uint8_t* t16 = tile16 + (yPos * 16 + xPos);

        for ([[maybe_unused]] const auto y : range(8)) {
            memcpy(t16, t8, 8);
            t8 += 8;
            t16 += 16;
        }
    };

    transform(tiles[0], 0, 0);
    transform(tiles[1], 8, 0);
    transform(tiles[2], 0, 8);
    transform(tiles[3], 8, 8);

    return ret;
}

}
