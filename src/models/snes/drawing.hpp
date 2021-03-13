/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "tile.h"
#include "tileset.h"
#include "models/common/image.h"
#include "models/common/iterators.h"

namespace UnTech::Snes {

template <size_t TS, typename PixelT>
inline void assertCanDrawTile(PixelT* imgBits, const PixelT* const imgBitsEnd, const size_t stride)
{
    const PixelT* tileEnd = imgBits + stride * (TS - 1) + TS;
    assert(tileEnd <= imgBitsEnd);
};

template <size_t TS, typename PixelT, typename DrawPixelFunction>
void drawTile_noFlip(const Tile<TS>& tile,
                     PixelT* imgBits, const PixelT* const imgBitsEnd, const size_t stride,
                     DrawPixelFunction drawPixel)
{
    assertCanDrawTile<TS>(imgBits, imgBitsEnd, stride);

    auto tilePos = tile.data().cbegin();

    for ([[maybe_unused]] const auto y : range(TS)) {
        for ([[maybe_unused]] const auto x : range(TS)) {
            drawPixel(*imgBits++, *tilePos++);
        }
        imgBits += (stride - TS);
    }
    assert(tilePos == tile.data().cend());
}

template <size_t TS, typename PixelT, typename DrawPixelFunction>
void drawTile_hFlip(const Tile<TS>& tile,
                    PixelT* imgBits, const PixelT* const imgBitsEnd, const size_t stride,
                    DrawPixelFunction drawPixel)
{
    assertCanDrawTile<TS>(imgBits, imgBitsEnd, stride);

    const auto& tileData = tile.data();

    for (const auto y : range(TS)) {
        for (const auto x : reverse_range(TS)) {
            drawPixel(*imgBits++, tileData[y * TS + x]);
        }
        imgBits += (stride - TS);
    }
}

template <size_t TS, typename PixelT, typename DrawPixelFunction>
void drawTile_vFlip(const Tile<TS>& tile,
                    PixelT* imgBits, const PixelT* const imgBitsEnd, const size_t stride,
                    DrawPixelFunction drawPixel)
{
    assertCanDrawTile<TS>(imgBits, imgBitsEnd, stride);

    const auto& tileData = tile.data();

    for (const auto y : reverse_range(TS)) {
        for (const auto x : range(TS)) {
            drawPixel(*imgBits++, tileData[y * TS + x]);
        }
        imgBits += (stride - TS);
    }
}

template <size_t TS, typename PixelT, typename DrawPixelFunction>
void drawTile_hvFlip(const Tile<TS>& tile,
                     PixelT* imgBits, const PixelT* const imgBitsEnd, const size_t stride,
                     DrawPixelFunction drawPixel)
{
    assertCanDrawTile<TS>(imgBits, imgBitsEnd, stride);

    auto tilePos = tile.data().crbegin();

    for ([[maybe_unused]] const auto y : range(TS)) {
        for ([[maybe_unused]] const auto x : range(TS)) {
            drawPixel(*imgBits++, *tilePos++);
        }
        imgBits += (stride - TS);
    }
    assert(tilePos == tile.data().crend());
}

template <size_t TS, typename PixelT, typename DrawPixelFunction>
inline void drawTile(const Tile<TS>& tile, const bool hFlip, const bool vFlip,
                     PixelT* imgBits, const PixelT* const imgBitsEnd, const size_t stride,
                     DrawPixelFunction drawPixel)
{
    if (!hFlip && !vFlip) {
        return drawTile_noFlip(tile, imgBits, imgBitsEnd, stride, drawPixel);
    }
    else if (hFlip && !vFlip) {
        return drawTile_hFlip(tile, imgBits, imgBitsEnd, stride, drawPixel);
    }
    else if (!hFlip && vFlip) {
        return drawTile_vFlip(tile, imgBits, imgBitsEnd, stride, drawPixel);
    }
    else if (hFlip && vFlip) {
        return drawTile_hvFlip(tile, imgBits, imgBitsEnd, stride, drawPixel);
    }
}

template <size_t TS, size_t BD>
void drawTile_transparent(const Tile<TS>& tile,
                          rgba* imgBits, const rgba* const imgBitsEnd, const size_t stride,
                          const Palette<BD>& palette)
{
    drawTile_noFlip(tile,
                    imgBits, imgBitsEnd, stride,
                    [&](rgba& img, uint8_t p) {
                        p &= palette.PIXEL_MASK;
                        if (p != 0) {
                            img = palette.color(p).rgb();
                        }
                    });
}

template <size_t TS, size_t BD>
void drawTile_transparent(const Tile<TS>& tile, const bool hFlip, const bool vFlip,
                          rgba* imgBits, const rgba* const imgBitsEnd, const size_t stride,
                          const Palette<BD>& palette)
{
    drawTile(tile, hFlip, vFlip,
             imgBits, imgBitsEnd, stride,
             [&](rgba& img, uint8_t p) {
                 p &= palette.PIXEL_MASK;
                 if (p != 0) {
                     img = palette.color(p).rgb();
                 }
             });
}

// Fails silently if offset is invalid
template <size_t TS, size_t BD>
inline void drawTile_transparent(const Tile<TS>& tile, const bool hFlip, const bool vFlip,
                                 const Palette<BD>& palette,
                                 Image& image, unsigned xOffset, unsigned yOffset)
{
    if ((xOffset + TS) < image.size().width
        && (yOffset + TS) < image.size().height) {

        drawTile_transparent(tile, hFlip, vFlip,
                             image.data() + yOffset * image.pixelsPerScanline() + xOffset,
                             image.data() + image.dataSize(),
                             image.pixelsPerScanline(),
                             palette);
    }
}

// Fails silently if tileId or offset is invalid
template <size_t TS, size_t BD>
inline void drawTile_transparent(const BaseTileset<TS>& tileset, unsigned tileId, const bool hFlip, const bool vFlip,
                                 const Palette<BD>& palette,
                                 Image& image, unsigned xOffset, unsigned yOffset)
{
    if (tileId < tileset.size()) {
        drawTile_transparent(tileset.at(tileId), hFlip, vFlip, palette, image, xOffset, yOffset);
    }
}

// NOTE: Image MUST be large enough to hold tileset.
// ASSUMES `imgBits` points to the start of an image scanline.
template <size_t TS, size_t BD>
void drawTileset_transparent(const BaseTileset<TS>& tileset,
                             rgba* imgBits, const rgba* const imgBitsEnd, const size_t stride,
                             const Palette<BD>& palette)
{
    if (tileset.empty()) {
        return;
    }

    static_assert(TS > 1);
    assert(stride % tileset.TILE_SIZE == 0);

    const size_t tilesPerLine = stride / tileset.TILE_SIZE;
    const size_t nLines = (tileset.size() - 1) / tilesPerLine + 1;

    assert(imgBitsEnd > imgBits);
    assert(size_t(imgBitsEnd - imgBits) >= stride * nLines * tileset.TILE_SIZE);

    unsigned x = 0;

    for (const Tile<TS>& tile : tileset) {
        Snes::drawTile_transparent(tile, imgBits, imgBitsEnd, stride, palette);

        imgBits += tile.TILE_SIZE;

        x++;
        if (x >= tilesPerLine) {
            x = 0;
            imgBits += stride * (tileset.TILE_SIZE - 1);
        }
    }
}

}
