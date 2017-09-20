/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/snes/tile.h"
#include <QImage>

namespace UnTech {
namespace GuiQt {
namespace Snes {

template <size_t TS, class PaletteT, bool showTransparent>
void drawTile(QImage& image,
              const UnTech::Snes::Tile<TS>& tile,
              const PaletteT& palette,
              unsigned xOffset, unsigned yOffset)
{
    const unsigned TILE_SIZE = tile.TILE_SIZE;

    if (image.width() < int(xOffset + TILE_SIZE)
        || image.height() < int(yOffset + TILE_SIZE)) {

        return;
    }

    const uint8_t* tileData = tile.rawData();
    for (unsigned y = 0; y < TILE_SIZE; y++) {
        QRgb* imgBits = reinterpret_cast<QRgb*>(image.scanLine(yOffset + y)) + xOffset;

        for (unsigned x = 0; x < TILE_SIZE; x++) {
            auto p = *tileData & palette.PIXEL_MASK;
            if (showTransparent || p != 0) {
                const UnTech::rgba& rgb = palette.color(p).rgb();
                *imgBits = qRgb(rgb.red, rgb.green, rgb.blue);
            }
            imgBits++;
            tileData++;
        }
    }
}

template <size_t TS, class PaletteT>
inline void drawOpaqueTile(QImage& image,
                           const UnTech::Snes::Tile<TS>& tile,
                           const PaletteT& palette,
                           unsigned xOffset, unsigned yOffset)
{
    drawTile<TS, PaletteT, true>(image, tile, palette, xOffset, yOffset);
}

template <size_t TS, class PaletteT>
inline void drawTransparentTile(QImage& image,
                                const UnTech::Snes::Tile<TS>& tile,
                                const PaletteT& palette,
                                unsigned xOffset, unsigned yOffset)
{
    drawTile<TS, PaletteT, false>(image, tile, palette, xOffset, yOffset);
}
}
}
}
