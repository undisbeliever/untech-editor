/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/enummap.h"
#include "models/metasprite/common.h"
#include <cstdint>

namespace UnTech::MetaSprite {

// Declared in `spriteimporter-serializer.cpp`
extern const EnumMap<TilesetType> tilesetTypeEnumMap;

inline const std::u8string& ttString(const TilesetType tt)
{
    return tilesetTypeEnumMap.nameOf(tt);
}

inline TilesetType smallestFixedTilesetType(unsigned tilesetSize)
{
    if (tilesetSize > 16) {
        throw invalid_argument(u8"too many tiles");
    }
    else if (tilesetSize > 8) {
        return TilesetType::TWO_ROWS_FIXED;
    }
    else if (tilesetSize > 2) {
        return TilesetType::ONE_ROW_FIXED;
    }
    else if (tilesetSize == 2) {
        return TilesetType::TWO_TILES_FIXED;
    }
    else {
        return TilesetType::ONE_TILE_FIXED;
    }
}

inline bool isFixedTilesetType(const TilesetType tt)
{
    switch (tt) {
    case TilesetType::ONE_TILE_FIXED:
    case TilesetType::TWO_TILES_FIXED:
    case TilesetType::ONE_ROW_FIXED:
    case TilesetType::TWO_ROWS_FIXED:
        return true;

    case TilesetType::ONE_TILE:
    case TilesetType::TWO_TILES:
    case TilesetType::ONE_ROW:
    case TilesetType::TWO_ROWS:
        return false;
    }

    abort();
}

inline uint8_t tilesetTypeRomValue(const TilesetType tt)
{
    return static_cast<uint8_t>(tt) * 2;
}

inline unsigned numberOfTilesetTiles(const TilesetType tt)
{
    switch (tt) {
    case TilesetType::ONE_TILE:
    case TilesetType::ONE_TILE_FIXED:
        return 1;

    case TilesetType::TWO_TILES:
    case TilesetType::TWO_TILES_FIXED:
        return 2;

    case TilesetType::ONE_ROW:
    case TilesetType::ONE_ROW_FIXED:
        return 8;

    case TilesetType::TWO_ROWS:
    case TilesetType::TWO_ROWS_FIXED:
        return 16;
    }

    abort();
}

inline unsigned tilesetSplitPoint(const TilesetType tt)
{
    switch (tt) {
    case TilesetType::ONE_TILE:
    case TilesetType::ONE_TILE_FIXED:
    case TilesetType::TWO_TILES:
    case TilesetType::TWO_TILES_FIXED:
        return 1;

    case TilesetType::ONE_ROW:
    case TilesetType::ONE_ROW_FIXED:
    case TilesetType::TWO_ROWS:
    case TilesetType::TWO_ROWS_FIXED:
        return 8;
    }

    abort();
}

}
