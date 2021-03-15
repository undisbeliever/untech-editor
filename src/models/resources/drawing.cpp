/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "drawing.h"
#include "models/snes/bit-depth.h"
#include "models/snes/drawing.hpp"

namespace UnTech::Resources {

static const Snes::Tile8px blankTile;
static const std::vector<Snes::Tile8px> blankTileset;

void drawAnimatedTileset(grid<uint8_t>& image, const Resources::AnimatedTilesetData& animatedTileset, unsigned frameIndex)
{
    constexpr unsigned TILE_SIZE = Snes::Tile8px::TILE_SIZE;

    // ::TODO add mode 0 palette offset::

    const auto& staticTiles = animatedTileset.staticTiles;
    const auto& animatedTiles = frameIndex < animatedTileset.animatedTiles.size() ? animatedTileset.animatedTiles.at(frameIndex) : blankTileset;
    const auto& tileMap = animatedTileset.tileMap;
    const unsigned colorsPerTile = Snes::colorsForBitDepth(animatedTileset.bitDepth);
    const unsigned pixelMask = Snes::pixelMaskForBitdepth(animatedTileset.bitDepth);

    assert(image.size() == usize(tileMap.width() * TILE_SIZE, tileMap.height() * TILE_SIZE));

    uint8_t* imgBits = &*image.begin();
    const uint8_t* const imgBitsEnd = imgBits + image.cellCount();
    const size_t stride = image.width();

    auto mapIt = tileMap.cbegin();

    for ([[maybe_unused]] const auto y : range(tileMap.height())) {
        for ([[maybe_unused]] const auto x : range(tileMap.width())) {
            const Snes::TilemapEntry& tm = *mapIt++;

            const Snes::Tile8px& tile = [&]() {
                const auto c = tm.character();
                if (c < staticTiles.size()) {
                    return staticTiles.at(c);
                }
                else if (c < staticTiles.size() + animatedTiles.size()) {
                    return animatedTiles.at(c - staticTiles.size());
                }
                return blankTile;
            }();

            const uint8_t palOffset = tm.palette() * colorsPerTile;

            Snes::drawTile(tile, tm.hFlip(), tm.vFlip(),
                           imgBits, imgBitsEnd, stride,
                           [&](uint8_t& pixel, uint8_t tc) {
                               tc &= pixelMask;
                               pixel = tc != 0 ? (tc + palOffset) & 0xff : 0;
                           });

            imgBits += TILE_SIZE;
        }

        imgBits += stride * (TILE_SIZE - 1);
    }
    assert(mapIt == tileMap.cend());
    assert(imgBits == imgBitsEnd);
}

}
