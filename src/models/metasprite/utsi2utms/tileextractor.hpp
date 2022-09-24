/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once
#include "models/common/iterators.h"
#include "models/metasprite/metasprite.h"
#include "models/metasprite/spriteimporter.h"
#include "models/snes/tile-data.h"
#include "models/snes/tilesetinserter.h"
#include <array>
#include <map>

namespace UnTech::MetaSprite::Utsi2UtmsPrivate {

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

class FrameConverter;

class TileExtractor {
private:
    const Image& image;
    const std::map<rgba, unsigned> colorMap;

public:
    Snes::TilesetInserter8px smallTileset;
    Snes::TilesetInserter16px largeTileset;

public:
    TileExtractor(MetaSprite::FrameSet& msFrameSet,
                  const Image& image,
                  const std::map<rgba, unsigned>& colorMap)
        : image(image)
        , colorMap(colorMap)
        , smallTileset(msFrameSet.smallTileset)
        , largeTileset(msFrameSet.largeTileset)
    {
        assert(!image.empty());
    }

    const Snes::TilesetInserterOutput getTilesetOutputFromImage(const urect& frameAabb, const SI::FrameObject& obj)
    {
        if (obj.size == ObjectSize::SMALL) {
            return smallTileset.getOrInsert(getTileFromImage<8>(frameAabb, obj));
        }
        else {
            return largeTileset.getOrInsert(getTileFromImage<16>(frameAabb, obj));
        }
    }

    template <size_t OVER_SIZE>
    Snes::TilesetInserterOutput getOrInsertTile(const Snes::Tile<OVER_SIZE>& tile);

    template <size_t TILE_SIZE>
    inline Snes::Tile<TILE_SIZE> getTileFromImage(const urect& frameAabb, const SI::FrameObject& obj) const
    {
        unsigned xOffset = frameAabb.x + obj.location.x;
        unsigned yOffset = frameAabb.y + obj.location.y;

        assert(yOffset + TILE_SIZE <= image.size().height);
        assert(xOffset + TILE_SIZE <= image.size().width);

        Snes::Tile<TILE_SIZE> tile;
        auto tData = tile.data().begin();

        for (const auto y : range(TILE_SIZE)) {
            const auto imgBits = image.scanline(yOffset + y).subspan(xOffset, TILE_SIZE);

            for (const auto& c : imgBits) {
                *tData++ = colorMap.at(c);
            }
        }
        assert(tData == tile.data().end());

        return tile;
    }
};

template <>
Snes::TilesetInserterOutput TileExtractor::getOrInsertTile<8>(const Snes::Tile8px& tile)
{
    return smallTileset.getOrInsert(tile);
}

template <>
Snes::TilesetInserterOutput TileExtractor::getOrInsertTile<16>(const Snes::Tile16px& tile)
{
    return largeTileset.getOrInsert(tile);
}

// mark the pixels in the undertile that are overlapped by the overtile
template <size_t OVER_SIZE, size_t UNDER_SIZE>
static std::array<bool, UNDER_SIZE * UNDER_SIZE>
markOverlappedPixels(const Snes::Tile<OVER_SIZE>& overTile,
                     int xOffset, int yOffset)
{
    static_assert(OVER_SIZE > 0 && OVER_SIZE < 64, "bad OVER_SIZE");
    static_assert(UNDER_SIZE > 0 && UNDER_SIZE < 64, "bad UNDER_SIZE");

    constexpr size_t RET_SIZE = UNDER_SIZE * UNDER_SIZE;

    constexpr int overSize = OVER_SIZE;
    constexpr int underSize = UNDER_SIZE;

    std::array<bool, RET_SIZE> ret = {};

    for (const auto oY : irange(overSize)) {
        int uY = oY + yOffset;

        if (uY >= 0 && uY < underSize) {
            for (const auto oX : irange(overSize)) {
                int uX = oX + xOffset;

                if (uX >= 0 && uX < underSize) {
                    if (overTile.pixel(oX, oY) != 0) {
                        ret[uY * underSize + uX] = true;
                    }
                }
            }
        }
    }

    return ret;
}

// clears the pixels in the overtile that match the undertile.
template <size_t OVER_SIZE, size_t UNDER_SIZE>
static void clearCommonOverlappedTiles(Snes::Tile<OVER_SIZE>& overTile,
                                       Snes::Tile<UNDER_SIZE>& underTile,
                                       int xOffset, int yOffset)
{
    static_assert(OVER_SIZE > 0 && OVER_SIZE < 64, "bad OVER_SIZE");
    static_assert(UNDER_SIZE > 0 && UNDER_SIZE < 64, "bad UNDER_SIZE");

    constexpr int overSize = OVER_SIZE;
    constexpr int underSize = UNDER_SIZE;

    for (const auto oY : irange(overSize)) {
        int uY = oY + yOffset;

        if (uY >= 0 && uY < underSize) {
            for (const auto oX : irange(overSize)) {
                int uX = oX + xOffset;

                if (uX >= 0 && uX < underSize) {
                    if (overTile.pixel(oX, oY) == underTile.pixel(uX, uY)) {
                        overTile.setPixel(oX, oY, 0);
                    }
                }
            }
        }
    }
}

}
