/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/grid.h"
#include "models/snes/tilemap.h"
#include "models/snes/tileset.h"
#include <vector>

namespace UnTech {
class ErrorList;

namespace Resources {

struct AnimatedTilesetData {
    constexpr static unsigned MAX_SNES_TILES = 1024;
    constexpr static unsigned MAX_ANIMATED_TILES_BLOCK_SIZE = 8192;
    constexpr static unsigned MAX_ANIMATED_TILES_FRAME_SIZE = 32 * 32;
    constexpr static unsigned ANIMATION_FRAME_SIZE_SCALE = 16;

    const static int ANIMATED_TILESET_FORMAT_VERSION;

    AnimatedTilesetData(int bitDepth)
        : staticTiles(bitDepth)
    {
    }

    Snes::Tileset8px staticTiles;
    std::vector<Snes::Tileset8px> animatedTiles;

    unsigned conversionPaletteIndex;

    unsigned animationDelay;

    grid<Snes::TilemapEntry> tileMap;

    unsigned nAnimatedFrames() const;

    unsigned nAnimatedTiles() const;
    unsigned animatedTilesFrameSize() const;
    unsigned animatedTilesBlockSize() const;

    // number of bytes of VRAM required to hold the tile data
    unsigned vramTileSize() const;

    bool validate(ErrorList& err) const;

    // Does not export tileMap.
    // Expects this AnimatedTilesetData to be valid.
    std::vector<uint8_t> exportAnimatedTileset() const;
};
}
}
