/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tile.h"
#include <cstring>

namespace UnTech {
namespace Snes {

template <size_t TS>
template <class PaletteT>
void Tile<TS>::draw(Image& image, const PaletteT& palette,
                    unsigned xOffset, unsigned yOffset,
                    const bool hFlip, const bool vFlip) const
{
    if (image.size().width < (xOffset + TILE_SIZE)
        || image.size().height < (yOffset + TILE_SIZE)) {

        return;
    }

    rgba* imgBits;
    const uint8_t* tilePos = rawData();
    for (unsigned y = 0; y < TILE_SIZE; y++) {
        unsigned fy = (vFlip == false) ? y : TILE_SIZE - 1 - y;
        imgBits = image.scanline(yOffset + fy) + xOffset;

        for (unsigned x = 0; x < TILE_SIZE; x++) {
            unsigned fx = (hFlip == false) ? x : TILE_SIZE - 1 - x;

            auto p = *tilePos & palette.PIXEL_MASK;
            if (p != 0) {
                imgBits[fx] = palette.color(p).rgb();
            }
            tilePos++;
        }
    }
}

template <size_t TS>
template <class PaletteT>
void Tile<TS>::drawOpaque(Image& image, const PaletteT& palette,
                          unsigned xOffset, unsigned yOffset,
                          const bool hFlip, const bool vFlip) const
{
    if (image.size().width < (xOffset + TILE_SIZE)
        || image.size().height < (yOffset + TILE_SIZE)) {

        return;
    }

    rgba* imgBits;
    const uint8_t* tilePos = rawData();
    for (unsigned y = 0; y < TILE_SIZE; y++) {
        unsigned fy = (vFlip == false) ? y : TILE_SIZE - 1 - y;
        imgBits = image.scanline(yOffset + fy) + xOffset;

        for (unsigned x = 0; x < TILE_SIZE; x++) {
            unsigned fx = (hFlip == false) ? x : TILE_SIZE - 1 - x;
            auto p = *tilePos & palette.PIXEL_MASK;
            imgBits[fx] = palette.color(p).rgb();
            tilePos++;
        }
    }
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

template <size_t TS>
Tile<TS> Tile<TS>::flip(bool hFlip, bool vFlip) const
{
    Tile<TS> ret;
    const uint8_t* pixelData = this->rawData();

    for (unsigned y = 0; y < TS; y++) {
        unsigned fy = (vFlip == false) ? y : TS - 1 - y;
        uint8_t* retRow = ret.rawData() + fy * TS;

        for (unsigned x = 0; x < TS; x++) {
            unsigned fx = (hFlip == false) ? x : TS - 1 - x;
            retRow[fx] = *pixelData++;
        }
    }

    return ret;
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

        for (unsigned y = 0; y < 8; y++) {
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

        for (unsigned y = 0; y < 8; y++) {
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
}
