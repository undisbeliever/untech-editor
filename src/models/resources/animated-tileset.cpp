/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animated-tileset.h"
#include "models/common/bytevectorhelper.h"
#include "models/common/errorlist.h"
#include "models/lz4/lz4.h"
#include "models/snes/animatedtilesetinserter.h"
#include "models/snes/bit-depth.h"
#include "models/snes/tile-data.h"
#include "models/snes/tilesetinserter.h"
#include <algorithm>
#include <cassert>

namespace UnTech {
namespace Resources {

unsigned AnimatedTilesetData::nAnimatedFrames() const
{
    if (!animatedTiles.empty()) {
        return animatedTiles.size();
    }
    else {
        return 1;
    }
}

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
    return nAnimatedTiles() * Snes::snesTileSizeForBitdepth(bitDepth);
}

unsigned AnimatedTilesetData::animatedTilesBlockSize() const
{
    return animatedTiles.size() * nAnimatedTiles() * Snes::snesTileSizeForBitdepth(bitDepth);
}

unsigned AnimatedTilesetData::vramTileSize() const
{
    return (staticTiles.size() + nAnimatedTiles()) * Snes::snesTileSizeForBitdepth(bitDepth);
}

bool AnimatedTilesetData::validate(ErrorList& err) const
{
    bool valid = true;

    for (const auto& at : animatedTiles) {
        if (at.size() != nAnimatedTiles()) {

            err.addErrorString("animatedTiles is invalid");
            valid = false;
        }
    }

    if (staticTiles.size() == 0) {
        err.addErrorString("Expected at least one static tile");
        valid = false;
    }

    auto validateMax = [&](unsigned v, unsigned max, const char* msg) {
        if (v > max) {
            err.addErrorString(msg, " (", v, ", max: ", max, ")");
            valid = false;
        }
    };
    validateMax(animatedTilesBlockSize(), MAX_ANIMATED_TILES_BLOCK_SIZE, "Too many animated tiles");
    validateMax(animatedTilesFrameSize(), MAX_ANIMATED_TILES_FRAME_SIZE, "Animated frame size too large");
    validateMax(staticTiles.size() + nAnimatedTiles(), MAX_SNES_TILES, "Too many tiles");

    return valid;
}

const int AnimatedTilesetData::ANIMATED_TILESET_FORMAT_VERSION = 2;

std::vector<uint8_t> AnimatedTilesetData::exportAnimatedTileset() const
{
    std::vector<uint8_t> block = Snes::snesTileData(staticTiles, bitDepth);
    block.reserve(block.size() + animatedTilesBlockSize());

    for (const auto& at : animatedTiles) {
        assert(at.size() == animatedTiles.front().size());

        const std::vector<uint8_t> atData = Snes::snesTileData(at, bitDepth);
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
