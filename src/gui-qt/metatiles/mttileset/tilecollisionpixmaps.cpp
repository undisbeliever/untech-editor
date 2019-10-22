/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilecollisionpixmaps.h"
#include <array>

namespace UnTech::GuiQt::MetaTiles::MtTileset {

constexpr auto METATILE_SIZE_PX = MT::METATILE_SIZE_PX;
constexpr auto TEXTURE_WIDTH = TILE_COLLISION_TEXTURE_WIDTH;
constexpr auto TEXTURE_HEIGHT = MT::METATILE_SIZE_PX;

constexpr size_t QIMAGE_ALIGNMENT = 32 / 8;

// clang-format off
constexpr static std::array<uint8_t, 16 * 16>
// NOTE: If you change these values, you must also change them in untech-engine
topHeightTable = {
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10,  9,  9,  8,  8,
     7,  7,  6,  6,  5,  5,  4,  4,  3,  3,  2,  2,  1,  1,  0,  0,
     0,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  7,  7,
     8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
},
bottomHeightTable = {
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
     0,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  7,  7,
     8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15,
    15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10,  9,  9,  8,  8,
     7,  7,  6,  6,  5,  5,  4,  4,  3,  3,  2,  2,  1,  1,  0,  0,
};
// clang-format on

constexpr unsigned _lengthOfHeightTable(unsigned x)
{
    return bottomHeightTable.at(x) - topHeightTable.at(x);
}

constexpr unsigned TEXTURE_BYTES_PER_LINE = unsigned((TEXTURE_WIDTH / 8 + QIMAGE_ALIGNMENT - 1) / QIMAGE_ALIGNMENT) * QIMAGE_ALIGNMENT;

alignas(QIMAGE_ALIGNMENT) constexpr static std::array<uint8_t, TEXTURE_HEIGHT* TEXTURE_BYTES_PER_LINE> collisionTextureBitmap = []() {
    // std::array<bool> annoyingly does not pack bit, have to use uint8_t instead
    std::array<uint8_t, TEXTURE_HEIGHT * TEXTURE_BYTES_PER_LINE> bitmap{};

    for (unsigned tx = 0; tx < TEXTURE_WIDTH; tx += METATILE_SIZE_PX) {
        MT::TileCollision tc = MT::TileCollision(tx / METATILE_SIZE_PX);

        auto top = [&](const unsigned x) -> unsigned { return topHeightTable.at(tx + x) < METATILE_SIZE_PX ? topHeightTable.at(tx + x) : 0; };
        auto bottom = [&](const unsigned x) -> unsigned { return bottomHeightTable.at(tx + x) < METATILE_SIZE_PX ? bottomHeightTable.at(tx + x) : 15; };

        auto setTilePixel = [&](const unsigned x, const unsigned y) {
            if (y < METATILE_SIZE_PX) {
                bitmap.at(y * TEXTURE_BYTES_PER_LINE + (tx + x) / 8) |= uint8_t(1U << (x & 7));
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

        if (tc == MT::TileCollision::EMPTY) {
            continue;
        }
        else if (tc == MT::TileCollision::SOLID) {
            for (unsigned y = 0; y < METATILE_SIZE_PX; y++) {
                for (unsigned x = 0; x < METATILE_SIZE_PX; x++) {
                    setTilePixel(x, y);
                }
            }
        }
        else if (tc == MT::TileCollision::UP_PLATFORM) {
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
        else if (tc == MT::TileCollision::DOWN_PLATFORM) {
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
        else if (tc == MT::TileCollision::END_SLOPE) {
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

    return bitmap;
}();

QPixmap TileCollisionTexture::createPixmap(MT::TileCollision tc, QColor color)
{
    QImage image(collisionTextureBitmap.data(), TEXTURE_WIDTH, TEXTURE_HEIGHT, TEXTURE_BYTES_PER_LINE, QImage::Format_MonoLSB);
    image = image.copy(xOffset(tc), 0, METATILE_SIZE_PX, METATILE_SIZE_PX);
    image.setColor(0, qRgba(0, 0, 0, 0));
    image.setColor(1, color.rgba());

    return QPixmap::fromImage(image);
}

QBitmap TileCollisionTexture::createBitmap()
{
    static_assert(TEXTURE_BYTES_PER_LINE % QIMAGE_ALIGNMENT == 0);
    Q_ASSERT(reinterpret_cast<size_t>(collisionTextureBitmap.data()) % QIMAGE_ALIGNMENT == 0);

    // Qt 5.12 QBitmap::fromImage internally converts image format to QImage::Format_MonoLSB.
    QImage image(collisionTextureBitmap.data(), TEXTURE_WIDTH, TEXTURE_HEIGHT, TEXTURE_BYTES_PER_LINE, QImage::Format_MonoLSB);
    return QBitmap::fromImage(std::move(image));
}

}
