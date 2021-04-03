/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/grid.h"
#include "models/snes/tile.h"
#include "models/snes/tilemap.h"
#include <vector>

namespace UnTech {
class ErrorList;

namespace Project {
template <typename T>
class DataStore;
}

namespace Resources {

struct PaletteInput;
struct PaletteData;

struct AnimationFramesInput {
    std::vector<std::filesystem::path> frameImageFilenames;

    // Palette used in tileset convertion, may not be the palette used on screen.
    idstring conversionPalette;

    unsigned animationDelay = 30;

    unsigned bitDepth = 4;
    bool addTransparentTile = false;

    bool isBitDepthValid() const;

    bool operator==(const AnimationFramesInput& o) const
    {
        return frameImageFilenames == o.frameImageFilenames
               && conversionPalette == o.conversionPalette
               && animationDelay == o.animationDelay
               && bitDepth == o.bitDepth
               && addTransparentTile == o.addTransparentTile;
    }
    bool operator!=(const AnimationFramesInput& o) const { return !(*this == o); }
};

struct AnimatedTilesetData {
    constexpr static unsigned MAX_SNES_TILES = 1024;
    constexpr static unsigned MAX_ANIMATED_TILES_BLOCK_SIZE = 8192;
    constexpr static unsigned MAX_ANIMATED_TILES_FRAME_SIZE = 32 * 32;
    constexpr static unsigned ANIMATION_FRAME_SIZE_SCALE = 16;

    const static int ANIMATED_TILESET_FORMAT_VERSION;

    std::vector<Snes::Tile8px> staticTiles;
    std::vector<std::vector<Snes::Tile8px>> animatedTiles;
    unsigned bitDepth;

    unsigned conversionPaletteIndex;

    unsigned animationDelay;

    grid<Snes::TilemapEntry> tileMap;

    unsigned nAnimatedFrames() const;

    unsigned nAnimatedTiles() const;
    unsigned animatedTilesFrameSize() const;
    unsigned animatedTilesBlockSize() const;

    // number of bytes of VRAM required to hold the tile data
    unsigned vramTileSize() const;

    // Does not export tileMap.
    // Expects this AnimatedTilesetData to be valid.
    std::vector<uint8_t> exportAnimatedTileset() const;
};

std::optional<AnimatedTilesetData>
convertAnimationFrames(const AnimationFramesInput& input,
                       const Project::DataStore<PaletteData>& projectDataStore,
                       ErrorList& err);

}
}
