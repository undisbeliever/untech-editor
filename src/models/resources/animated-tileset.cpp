/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animated-tileset.h"
#include "resources.h"
#include "models/common/bytevectorhelper.h"
#include "models/common/validatorhelper.h"
#include "models/lz4/lz4.h"
#include "models/snes/animatedtilesetinserter.h"
#include "models/snes/tilesetinserter.h"
#include <algorithm>
#include <cassert>

namespace UnTech {
namespace Resources {

unsigned AnimatedTilesetData::nAnimatedTiles() const
{
    if (!animatedTiles.empty()) {
        return animatedTiles.front().size();
    }
    else {
        return 0;
    }
}

unsigned AnimatedTilesetData::animatedTilesFrameSize() const
{
    return nAnimatedTiles() * staticTiles.snesTileSize();
}

unsigned AnimatedTilesetData::animatedTilesBlockSize() const
{
    return animatedTiles.size() * nAnimatedTiles() * staticTiles.snesTileSize();
}

void AnimatedTilesetData::validate() const
{
    if (mapHeight * mapWidth != tileMap.size()) {
        throw std::logic_error("Invalid tileMap");
    }

    for (const auto& at : animatedTiles) {
        if (at.size() != nAnimatedTiles()
            || at.bitDepth() != staticTiles.bitDepth()) {

            throw std::logic_error("animatedTiles is invalid");
        }
    }

    if (staticTiles.size() == 0) {
        throw std::runtime_error("Expected at least one static tile");
    }

    validateMax(animatedTilesBlockSize(), MAX_ANIMATED_TILES_BLOCK_SIZE, "Too many animated tiles");
    validateMax(animatedTilesFrameSize(), MAX_ANIMATED_TILES_FRAME_SIZE, "Animated frame size too large");
    validateMax(staticTiles.size() + nAnimatedTiles(), MAX_SNES_TILES, "Too many tiles");
}

const unsigned AnimatedTilesetData::ANIMATED_TILESET_FORMAT_VERSION = 2;

std::vector<uint8_t> AnimatedTilesetData::exportAnimatedTileset() const
{
    validate();

    std::vector<uint8_t> block = staticTiles.snesData();
    block.reserve(block.size() + animatedTilesBlockSize());

    for (const auto& at : animatedTiles) {
        const std::vector<uint8_t> atData = at.snesData();
        block.insert(block.end(), atData.begin(), atData.end());
    }
    block = lz4HcCompress(block);

    std::vector<uint8_t> out;
    out.reserve(block.size() + 6);

    if (animatedTiles.size() > 1) {
        assert(animatedTilesFrameSize() % ANIMATION_FRAME_SIZE_SCALE == 0);

        writeUint8(out, animatedTiles.size());
        writeUint8(out, animatedTilesFrameSize() / ANIMATION_FRAME_SIZE_SCALE);
        writeUint16(out, animationDelay);
    }
    else {
        writeUint8(out, 0);
    }

    out.insert(out.end(), block.begin(), block.end());

    return out;
}
}
}
