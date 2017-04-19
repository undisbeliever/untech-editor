#pragma once
#include "models/metasprite/metasprite.h"
#include "models/metasprite/spriteimporter.h"
#include "models/snes/tileset.h"
#include "models/snes/tilesetinserter.h"
#include <array>
#include <map>

namespace UnTech {
namespace MetaSprite {
namespace Utsi2UtmsPrivate {

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

class FrameConverter;

class TileExtractor {
    friend class FrameConverter;

    const Image& image;
    const std::map<rgba, unsigned> colorMap;
    const SpriteImporter::FrameSet& siFrameSet;
    MetaSprite::FrameSet& msFrameSet;
    ErrorList& errorList;

    Snes::TilesetInserter8px smallTileset;
    Snes::TilesetInserter16px largeTileset;

public:
    TileExtractor(const SpriteImporter::FrameSet& siFrameSet,
                  MetaSprite::FrameSet& msFrameSet,
                  ErrorList& errorList,
                  const std::map<rgba, unsigned>& colorMap)
        : image(*siFrameSet.image)
        , colorMap(colorMap)
        , siFrameSet(siFrameSet)
        , msFrameSet(msFrameSet)
        , errorList(errorList)
        , smallTileset(msFrameSet.smallTileset)
        , largeTileset(msFrameSet.largeTileset)
    {
        assert(siFrameSet.isImageValid());
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
    };

    template <size_t OVER_SIZE>
    Snes::TilesetInserterOutput getOrInsertTile(const Snes::Tile<OVER_SIZE>& tile);

    template <size_t TILE_SIZE>
    inline Snes::Tile<TILE_SIZE> getTileFromImage(const SI::Frame& frame,
                                                  const SI::FrameObject& obj)
    {
        unsigned xOffset = frame.location.aabb.x + obj.location.x;
        unsigned yOffset = frame.location.aabb.y + obj.location.y;

        Snes::Tile<TILE_SIZE> tile;
        uint8_t* tData = tile.rawData();
        for (unsigned y = 0; y < TILE_SIZE; y++) {
            const rgba* imgBits = image.scanline(yOffset + y) + xOffset;

            for (unsigned x = 0; x < TILE_SIZE; x++) {
                *tData++ = colorMap.at(*imgBits++);
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
inline std::array<bool, UNDER_SIZE * UNDER_SIZE>
markOverlappedPixels(const Snes::Tile<OVER_SIZE>& overTile,
                     int xOffset, int yOffset)
{
    const uint8_t* overTileData = overTile.rawData();

    std::array<bool, UNDER_SIZE* UNDER_SIZE> ret = {};

    for (unsigned oY = 0; oY < OVER_SIZE; oY++) {
        int uY = oY + yOffset;

        if (uY >= 0 && uY < (int)UNDER_SIZE) {
            for (unsigned oX = 0; oX < OVER_SIZE; oX++) {
                int uX = oX + xOffset;

                if (uX >= 0 && uX < (int)UNDER_SIZE) {
                    if (overTileData[oY * OVER_SIZE + oX] != 0) {
                        ret[uY * UNDER_SIZE + uX] = true;
                    }
                }
            }
        }
    }

    return ret;
}

// clears the pixels in the overtile that match the undertile.
template <size_t OVER_SIZE, size_t UNDER_SIZE>
inline void clearCommonOverlappedTiles(Snes::Tile<OVER_SIZE>& overTile,
                                       Snes::Tile<UNDER_SIZE>& underTile,
                                       int xOffset, int yOffset)
{
    uint8_t* overTileData = overTile.rawData();
    uint8_t* underTileData = underTile.rawData();

    for (unsigned oY = 0; oY < OVER_SIZE; oY++) {
        int uY = oY + yOffset;

        if (uY >= 0 && uY < (int)UNDER_SIZE) {
            for (unsigned oX = 0; oX < OVER_SIZE; oX++) {
                int uX = oX + xOffset;

                if (uX >= 0 && uX < (int)UNDER_SIZE) {
                    if (overTileData[oY * OVER_SIZE + oX] == underTileData[uY * UNDER_SIZE + uX]) {
                        overTileData[oY * OVER_SIZE + oX] = 0;
                    }
                }
            }
        }
    }
}
}
}
}
