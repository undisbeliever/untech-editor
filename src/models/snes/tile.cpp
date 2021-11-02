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
Tile<TS> Tile<TS>::hFlip() const
{
    Tile<TS> ret;
    auto retIt = ret.data().begin();

    for (const auto y : range(TS)) {
        const auto s = this->sliver(y);
        retIt = std::copy(s.rbegin(), s.rend(), retIt);
    }
    assert(retIt == ret.data().cend());

    return ret;
}

template <size_t TS>
Tile<TS> Tile<TS>::vFlip() const
{
    Tile<TS> ret;
    auto retIt = ret.data().begin();

    for (const auto y : reverse_range(TS)) {
        const auto s = this->sliver(y);
        retIt = std::copy(s.begin(), s.end(), retIt);
    }
    assert(retIt == ret.data().cend());

    return ret;
}

template <size_t TS>
Tile<TS> Tile<TS>::hvFlip() const
{
    Tile<TS> ret;

    std::copy(_data.crbegin(), _data.crend(), ret._data.begin());

    return ret;
}

template <std::size_t TS>
Tile<TS> Tile<TS>::flip(bool h_flip, bool v_flip) const
{
    if (!h_flip) {
        if (!v_flip) {
            return *this;
        }
        else {
            return vFlip();
        }
    }
    else {
        if (!v_flip) {
            return hFlip();
        }
        else {
            return hvFlip();
        }
    }
}

template class Snes::Tile<8>;
template class Snes::Tile<16>;

std::array<Tile8px, 4> splitLargeTile(const Tile16px& largeTile)
{
    std::array<Tile8px, 4> ret;

    auto transform = [&largeTile](Tile8px& leftTile, Tile8px& rightTile,
                                  unsigned yPos) {
        auto lIt = leftTile.data().begin();
        auto rIt = rightTile.data().begin();

        for (const auto y : range(8)) {
            const auto sliver = largeTile.sliver(y + yPos);

            const auto lSliver = sliver.first(8);
            lIt = std::copy(lSliver.begin(), lSliver.end(), lIt);

            const auto rSliver = sliver.subspan(8, 8);
            rIt = std::copy(rSliver.begin(), rSliver.end(), rIt);
        }
        assert(lIt == leftTile.data().end());
        assert(rIt == rightTile.data().end());
    };

    transform(ret[0], ret[1], 0);
    transform(ret[2], ret[3], 8);

    return ret;
}

Tile16px combineSmallTiles(const std::array<Tile8px, 4>& tiles)
{
    Tile<16> ret;

    auto transform = [&ret](const Tile8px& tile8,
                            unsigned xPos, unsigned yPos) {
        auto retIt = ret.data().begin() + yPos * ret.TILE_SIZE + xPos;

        for (const auto y : range(8)) {
            const auto sliver = tile8.sliver(y);

            std::copy(sliver.begin(), sliver.end(), retIt);

            retIt += ret.TILE_SIZE;
        }
    };

    transform(tiles[0], 0, 0);
    transform(tiles[1], 8, 0);
    transform(tiles[2], 0, 8);
    transform(tiles[3], 8, 8);

    return ret;
}

}
