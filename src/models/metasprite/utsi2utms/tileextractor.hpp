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

    const Snes::TilesetInserterOutput getTilesetOutputFromImage(const SI::Frame& frame,
                                                                const SI::FrameObject& obj)
    {
        if (obj.size == ObjectSize::SMALL) {
            return smallTileset.getOrInsert(getTileFromImage<8>(frame, obj));
        }
        else {
            return largeTileset.getOrInsert(getTileFromImage<16>(frame, obj));
        }
    }

    template <size_t OVER_SIZE>
    Snes::TilesetInserterOutput getOrInsertTile(const Snes::Tile<OVER_SIZE>& tile);

    template <size_t TILE_SIZE>
    inline Snes::Tile<TILE_SIZE> getTileFromImage(const SI::Frame& frame,
                                                  const SI::FrameObject& obj) const
    {
        unsigned xOffset = frame.location.aabb.x + obj.location.x;
        unsigned yOffset = frame.location.aabb.y + obj.location.y;

        Snes::Tile<TILE_SIZE> tile;
        uint8_t* tData = tile.rawData();

        for (const auto y : range(TILE_SIZE)) {
            const rgba* const imgBits = image.scanline(yOffset + y) + xOffset;

            for (const auto x : range(TILE_SIZE)) {
                *tData++ = colorMap.at(imgBits[x]);
            }
        }

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

    constexpr int overSize = OVER_SIZE;
    constexpr int underSize = UNDER_SIZE;

    const uint8_t* overTileData = overTile.rawData();

    std::array<bool, underSize* underSize> ret = {};

    for (const auto oY : irange(overSize)) {
        int uY = oY + yOffset;

        if (uY >= 0 && uY < underSize) {
            for (const auto oX : irange(overSize)) {
                int uX = oX + xOffset;

                if (uX >= 0 && uX < underSize) {
                    if (overTileData[oY * overSize + oX] != 0) {
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

    uint8_t* overTileData = overTile.rawData();
    uint8_t* underTileData = underTile.rawData();

    for (const auto oY : irange(overSize)) {
        int uY = oY + yOffset;

        if (uY >= 0 && uY < underSize) {
            for (const auto oX : irange(overSize)) {
                int uX = oX + xOffset;

                if (uX >= 0 && uX < underSize) {
                    if (overTileData[oY * overSize + oX] == underTileData[uY * underSize + uX]) {
                        overTileData[oY * overSize + oX] = 0;
                    }
                }
            }
        }
    }
}

}
