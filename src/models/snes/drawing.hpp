/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "tile.h"
#include "models/common/bit.h"
#include "models/common/image.h"
#include "models/common/iterators.h"

namespace UnTech::Snes {

inline constexpr unsigned pixelMaskForPaletteSize(const unsigned paletteSize)
{
    if (!isPowerOfTwo(paletteSize)) {
        throw invalid_argument("paletteSize not power of two");
    }

    return paletteSize - 1;
}

template <size_t TS, typename ImageT>
inline void assertCanDrawTile(const ImageT& image, const unsigned x, const unsigned y)
{
    assert(x + TS <= image.size().width && y + TS <= image.size().height);
};

template <size_t TS, typename ImageT, typename DrawPixelFunction>
void drawTile_noFlip(ImageT& image, const unsigned xOffset, const unsigned yOffset,
                     const Tile<TS>& tile, DrawPixelFunction drawPixel)
{
    assertCanDrawTile<TS>(image, xOffset, yOffset);

    auto tilePos = tile.data().cbegin();

    for (const auto y : range(TS)) {
        auto imgBits = image.scanline(yOffset + y).subspan(xOffset, TS);

        for (const auto x : range(TS)) {
            drawPixel(imgBits[x], *tilePos++);
        }
    }
    assert(tilePos == tile.data().cend());
}

template <size_t TS, typename ImageT, typename DrawPixelFunction>
void drawTile_hFlip(ImageT& image, const unsigned xOffset, const unsigned yOffset,
                    const Tile<TS>& tile, DrawPixelFunction drawPixel)
{
    assertCanDrawTile<TS>(image, xOffset, yOffset);

    auto tilePos = tile.data().cbegin();

    for (const auto y : range(TS)) {
        auto imgBits = image.scanline(y + yOffset).subspan(xOffset, TS);

        for (const auto x : reverse_range(TS)) {
            drawPixel(imgBits[x], *tilePos++);
        }
    }
    assert(tilePos == tile.data().cend());
}

template <size_t TS, typename ImageT, typename DrawPixelFunction>
void drawTile_vFlip(ImageT& image, const unsigned xOffset, const unsigned yOffset,
                    const Tile<TS>& tile, DrawPixelFunction drawPixel)
{
    assertCanDrawTile<TS>(image, xOffset, yOffset);

    auto tilePos = tile.data().cbegin();

    for (const auto y : reverse_range(TS)) {
        auto imgBits = image.scanline(y + yOffset).subspan(xOffset, TS);

        for (const auto x : range(TS)) {
            drawPixel(imgBits[x], *tilePos++);
        }
    }
    assert(tilePos == tile.data().cend());
}

template <size_t TS, typename ImageT, typename DrawPixelFunction>
void drawTile_hvFlip(ImageT& image, const unsigned xOffset, const unsigned yOffset,
                     const Tile<TS>& tile, DrawPixelFunction drawPixel)
{
    assertCanDrawTile<TS>(image, xOffset, yOffset);

    auto tilePos = tile.data().crbegin();

    for (const auto y : range(TS)) {
        auto imgBits = image.scanline(yOffset + y).subspan(xOffset, TS);

        for (const auto x : range(TS)) {
            drawPixel(imgBits[x], *tilePos++);
        }
    }
    assert(tilePos == tile.data().crend());
}

template <size_t TS, typename ImageT, typename DrawPixelFunction>
inline void drawTile(ImageT& image, const unsigned xOffset, const unsigned yOffset,
                     const Tile<TS>& tile, const bool hFlip, const bool vFlip,
                     DrawPixelFunction drawPixel)
{
    if (!hFlip && !vFlip) {
        return drawTile_noFlip(image, xOffset, yOffset, tile, drawPixel);
    }
    else if (hFlip && !vFlip) {
        return drawTile_hFlip(image, xOffset, yOffset, tile, drawPixel);
    }
    else if (!hFlip && vFlip) {
        return drawTile_vFlip(image, xOffset, yOffset, tile, drawPixel);
    }
    else if (hFlip && vFlip) {
        return drawTile_hvFlip(image, xOffset, yOffset, tile, drawPixel);
    }
}

template <size_t TS, size_t PS>
void drawTile_transparent(Image& image, const unsigned xOffset, const unsigned yOffset,
                          const Tile<TS>& tile,
                          const std::array<rgba, PS>& palette)
{
    constexpr unsigned PIXEL_MASK = pixelMaskForPaletteSize(PS);

    drawTile_noFlip(image, xOffset, yOffset,
                    tile,
                    [&](rgba& img, uint8_t p) {
                        p &= PIXEL_MASK;
                        if (p != 0) {
                            img = palette.at(p);
                        }
                    });
}

template <size_t TS, size_t PS>
void drawTile_transparent(Image& image, const unsigned xOffset, const unsigned yOffset,
                          const Tile<TS>& tile, const bool hFlip, const bool vFlip,
                          const std::array<rgba, PS>& palette)
{
    constexpr unsigned PIXEL_MASK = pixelMaskForPaletteSize(PS);

    drawTile(image, xOffset, yOffset,
             tile, hFlip, vFlip,
             [&](rgba& img, uint8_t p) {
                 p &= PIXEL_MASK;
                 if (p != 0) {
                     img = palette.at(p);
                 }
             });
}

template <size_t TS>
inline void drawTile_transparent(Image& image, const unsigned xOffset, const unsigned yOffset,
                                 const Tile<TS>& tile,
                                 const std::span<const rgba> palette)
{
    const unsigned pixelMask = pixelMaskForPaletteSize(palette.size());

    drawTile_noFlip(image, xOffset, yOffset,
                    tile,
                    [&](rgba& img, uint8_t p) {
                        p &= pixelMask;
                        if (p != 0) {
                            img = palette[p];
                        }
                    });
}

// Fails silently if tileId or offset is invalid
template <size_t TS, size_t PS>
inline void drawTile_transparent_safe(Image& image, const unsigned xOffset, const unsigned yOffset,
                                      const std::vector<Tile<TS>>& tileset, unsigned tileId, const bool hFlip, const bool vFlip,
                                      const std::array<rgba, PS>& palette)
{
    if (tileId < tileset.size()) {
        if (xOffset + TS <= image.size().width) {
            if (yOffset + TS <= image.size().height) {
                drawTile_transparent(image, xOffset, yOffset, tileset.at(tileId), hFlip, vFlip, palette);
            }
        }
    }
}

// Image MUST be large enough to hold tileset.
template <size_t TS>
void drawTileset_transparent(Image& image, const unsigned yOffset,
                             const std::vector<Tile<TS>>& tileset,
                             const std::span<const rgba> palette)
{
    if (tileset.empty()) {
        return;
    }

    static_assert(TS > 1);

    const size_t width = image.size().width;
    const size_t tilesPerLine = width / TS;
    const size_t nLines = (tileset.size() - 1) / tilesPerLine + 1;

    assert(width % TS == 0);
    assert(yOffset + nLines * TS <= image.size().height);

    unsigned y = yOffset;
    unsigned x = 0;

    for (const Tile<TS>& tile : tileset) {
        drawTile_transparent(image, x, y, tile, palette);

        x += TS;
        if (x >= width) {
            x = 0;
            y += TS;
        }
    }
}

}
