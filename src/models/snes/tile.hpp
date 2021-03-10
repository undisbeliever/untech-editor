/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tile.h"
#include "models/common/iterators.h"
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
    for (const auto y : range(TILE_SIZE)) {
        unsigned fy = (vFlip == false) ? y : TILE_SIZE - 1 - y;
        imgBits = image.scanline(yOffset + fy) + xOffset;

        for (const auto x : range(TILE_SIZE)) {
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
    for (const auto y : range(TILE_SIZE)) {
        unsigned fy = (vFlip == false) ? y : TILE_SIZE - 1 - y;
        imgBits = image.scanline(yOffset + fy) + xOffset;

        for (const auto x : range(TILE_SIZE)) {
            unsigned fx = (hFlip == false) ? x : TILE_SIZE - 1 - x;
            auto p = *tilePos & palette.PIXEL_MASK;
            imgBits[fx] = palette.color(p).rgb();
            tilePos++;
        }
    }
}

}
}
