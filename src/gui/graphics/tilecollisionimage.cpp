/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilecollisionimage.h"
#include "models/metatiles/metatile-tileset.h"
#include <cmath>

namespace UnTech::Gui {

static constexpr unsigned METATILE_SIZE_PX = MetaTiles::METATILE_SIZE_PX;
static constexpr unsigned N_TILE_COLLISIONS = MetaTiles::N_TILE_COLLISONS;

static_assert(TILE_COLLISION_IMAGE_WIDTH == 1 * METATILE_SIZE_PX);
static_assert(TILE_COLLISION_IMAGE_HEIGHT >= MetaTiles::N_TILE_COLLISONS * METATILE_SIZE_PX);

// clang-format off
constexpr static std::array<uint8_t, 16 * 16>
// NOTE: If you change these values, you must also change them in untech-engine
topHeightTable = {
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10,  9,  9,  8,  8,
     7,  7,  6,  6,  5,  5,  4,  4,  3,  3,  2,  2,  1,  1,  0,  0,
     0,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  7,  7,
     8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
},
bottomHeightTable = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
     0,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  7,  7,
     8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15,
    15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10,  9,  9,  8,  8,
     7,  7,  6,  6,  5,  5,  4,  4,  3,  3,  2,  2,  1,  1,  0,  0,
};
// clang-format on

using TileCollisionImageData = std::array<rgba, TILE_COLLISION_IMAGE_WIDTH * TILE_COLLISION_IMAGE_HEIGHT>;

static constexpr TileCollisionImageData generateTileCollisionImageData()
{
    constexpr unsigned WIDTH = TILE_COLLISION_IMAGE_WIDTH;

    using TileCollisionType = MetaTiles::TileCollisionType;

    TileCollisionImageData imgData{};

    for (unsigned tci = 0; tci < N_TILE_COLLISIONS; tci++) {
        const TileCollisionType tc = TileCollisionType(tci);
        const unsigned tileYoffset = tci * METATILE_SIZE_PX;

        auto top = [&](const unsigned x) -> unsigned { return topHeightTable.at(tileYoffset + x) < METATILE_SIZE_PX ? topHeightTable.at(tileYoffset + x) : 0; };
        auto bottom = [&](const unsigned x) -> unsigned { return bottomHeightTable.at(tileYoffset + x) < METATILE_SIZE_PX ? bottomHeightTable.at(tileYoffset + x) : 15; };

        auto setTilePixel = [&](const unsigned x, const unsigned y) {
            if (y < METATILE_SIZE_PX) {
                imgData.at((tileYoffset + y) * WIDTH + x) = rgba(255, 255, 255, 255);
            }
        };
        auto drawTileVLine = [&](const unsigned y) {
            for (unsigned x = 0; x < METATILE_SIZE_PX; x++) {
                setTilePixel(x, y);
            }
        };
        auto drawTileHLine = [&](const unsigned x) {
            for (unsigned y = top(x); y < bottom(x); y++) {
                setTilePixel(x, y);
            }
        };
        auto drawTileBox = [&]() {
            for (unsigned i = 0; i < METATILE_SIZE_PX; i++) {
                setTilePixel(0, i);
                setTilePixel(METATILE_SIZE_PX - 1, i);
                setTilePixel(i, 0);
                setTilePixel(i, METATILE_SIZE_PX - 1);
            }
        };

        if (tc == TileCollisionType::EMPTY) {
            continue;
        }
        else if (tc == TileCollisionType::SOLID) {
            for (unsigned y = 0; y < METATILE_SIZE_PX; y++) {
                for (unsigned x = 0; x < METATILE_SIZE_PX; x++) {
                    setTilePixel(x, y);
                }
            }
        }
        else if (tc == TileCollisionType::UP_PLATFORM) {
            drawTileBox();
            for (unsigned x = 4; x < METATILE_SIZE_PX - 1; x += 7) {
                for (unsigned y = 2; y < METATILE_SIZE_PX - 3; y += 3) {
                    setTilePixel(x + 0, y);
                    setTilePixel(x - 1, y + 1);
                    setTilePixel(x + 1, y + 1);
                    setTilePixel(x - 2, y + 2);
                    setTilePixel(x + 2, y + 2);
                }
            }
        }
        else if (tc == TileCollisionType::DOWN_PLATFORM) {
            drawTileBox();
            for (unsigned x = 4; x < METATILE_SIZE_PX - 1; x += 7) {
                for (unsigned y = 2; y < METATILE_SIZE_PX - 3; y += 3) {
                    setTilePixel(x + 0, y + 2);
                    setTilePixel(x - 1, y + 1);
                    setTilePixel(x + 1, y + 1);
                    setTilePixel(x - 2, y);
                    setTilePixel(x + 2, y);
                }
            }
        }
        else if (tc == TileCollisionType::END_SLOPE) {
            drawTileVLine(0);
            drawTileVLine(METATILE_SIZE_PX - 1);
            for (unsigned y = 3; y < METATILE_SIZE_PX; y += 4) {
                drawTileVLine(y);
                drawTileVLine(y + 1);
            }
        }
        else {
            // draw outline
            for (unsigned x = 0; x < METATILE_SIZE_PX; x++) {
                setTilePixel(x, top(x));
                setTilePixel(x, bottom(x));
            }
            drawTileHLine(0);
            drawTileHLine(METATILE_SIZE_PX - 1);
        }
    }

    return imgData;
}

static Texture generateTileCollisionTexture()
{
    constexpr auto tileCollisionData = generateTileCollisionImageData();

    Image img(TILE_COLLISION_IMAGE_WIDTH, TILE_COLLISION_IMAGE_HEIGHT);
    assert(img.size().width * img.size().height == tileCollisionData.size());
    assert(img.data().size() == tileCollisionData.size());

    std::copy(tileCollisionData.begin(), tileCollisionData.end(), img.data().begin());

    return Texture::createFromImage(img);
}

const Texture& tileCollisionTypeTexture()
{
    static Texture texture = generateTileCollisionTexture();
    return texture;
}

};
