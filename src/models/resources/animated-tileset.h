/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/snes/tilemap.h"
#include "models/snes/tileset.h"
#include <vector>

namespace UnTech {
namespace Resources {

struct AnimatedTilesetData {
    constexpr static unsigned MAX_SNES_TILES = 1024;
    constexpr static unsigned MAX_ANIMATED_TILES_BLOCK_SIZE = 8192;
    constexpr static unsigned MAX_ANIMATED_TILES_FRAME_SIZE = 32 * 32;
    constexpr static unsigned ANIMATION_FRAME_SIZE_SCALE = 16;

    const static unsigned ANIMATED_TILESET_FORMAT_VERSION;

    AnimatedTilesetData(int bitDepth)
        : staticTiles(bitDepth)
    {
    }

    Snes::Tileset8px staticTiles;
    std::vector<Snes::Tileset8px> animatedTiles;

    unsigned animationDelay;

    // tileMap is not interlaced and/or padded.
    std::vector<Snes::TilemapEntry> tileMap;
    unsigned mapWidth;
    unsigned mapHeight;

    unsigned nAnimatedTiles() const;
    unsigned animatedTilesFrameSize() const;
    unsigned animatedTilesBlockSize() const;

    // raises an exception if invalid
    void validate() const;

    // Does not export tileMap.
    // Raises an exception if invalid.
    std::vector<uint8_t> exportAnimatedTileset() const;
};
}
}
