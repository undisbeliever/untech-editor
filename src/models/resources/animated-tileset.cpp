/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animated-tileset.h"
#include "palette.h"
#include "models/common/bytevectorhelper.h"
#include "models/common/errorlist.h"
#include "models/common/imagecache.h"
#include "models/lz4/lz4.h"
#include "models/project/project-data.h"
#include "models/snes/animatedtilesetinserter.h"
#include "models/snes/bit-depth.h"
#include "models/snes/tile-data.h"
#include "models/snes/tilesetinserter.h"
#include <algorithm>
#include <cassert>

#include "tile-extractor.hpp"

namespace UnTech::Resources {

using Tile8px = Snes::Tile8px;

struct AnimatedTilesetIntermediate {
    std::vector<std::vector<Tile8px>> animatedTiles; // [tileId][frameId]
    std::vector<Tile8px> staticTiles;

    struct TM {
        bool isAnimated;
        unsigned tile;
        unsigned palette;
    };
    std::vector<TM> tileMap;
};

static std::vector<std::vector<TileAndPalette>> tilesFromFrameImages(const AnimationFramesInput input,
                                                                     const std::vector<Snes::SnesColor>& palette,
                                                                     ErrorList& err)
{
    std::vector<std::vector<TileAndPalette>> frameTiles;
    frameTiles.reserve(input.frameImageFilenames.size());

    for (const auto [frameIndex, fn] : const_enumerate(input.frameImageFilenames)) {
        std::vector<InvalidImageTile> invalidTiles;

        const auto& image = ImageCache::loadPngImage(fn);
        frameTiles.emplace_back(
            tilesFromImage(*image, input.bitDepth, palette, 0, 8, invalidTiles));

        if (!invalidTiles.empty()) {
            err.addError(std::make_unique<InvalidImageError>(std::move(invalidTiles), frameIndex));
        }
    }

    return frameTiles;
}

static AnimatedTilesetIntermediate combineFrameTiles(
    const std::vector<std::vector<TileAndPalette>>& frameTiles, unsigned tileWidth,
    ErrorList& err)
{
    assert(!frameTiles.empty());
    const unsigned nTiles = frameTiles.front().size();
    for (const auto& tf : frameTiles) {
        assert(tf.size() == nTiles);
    }

    auto getTile = [&](unsigned frameId, unsigned tileId) -> const Tile8px& {
        return frameTiles.at(frameId).at(tileId).tile;
    };
    auto getPalette = [&](unsigned frameId, unsigned tileId) -> unsigned {
        return frameTiles.at(frameId).at(tileId).palette;
    };

    const unsigned nFrames = frameTiles.size();

    std::vector<InvalidImageTile> invalidTiles;

    AnimatedTilesetIntermediate ret;
    ret.animatedTiles.reserve(64);
    ret.tileMap.resize(nTiles);

    for (auto [tileId, tm] : enumerate(ret.tileMap)) {
        const static unsigned TS = Tile8px::TILE_SIZE;

        unsigned palette = getPalette(0, tileId);
        for (const auto frameId : range(nFrames)) {
            if (getPalette(frameId, tileId) != palette) {
                invalidTiles.emplace_back(TS, (tileId % tileWidth) * TS, (tileId / tileWidth) * TS, InvalidTileReason::NOT_SAME_PALETTE);
                break;
            }
        }
        tm.palette = palette;

        const Tile8px& firstTile = getTile(0, tileId);
        bool isAnimated = false;
        for (const auto frameId : range(nFrames)) {
            if (getTile(frameId, tileId) != firstTile) {
                isAnimated = true;
                break;
            }
        }

        tm.isAnimated = isAnimated;
        if (isAnimated == false) {
            tm.tile = ret.staticTiles.size();
            ret.staticTiles.emplace_back(firstTile);
        }
        else {
            tm.tile = ret.animatedTiles.size();

            ret.animatedTiles.emplace_back(nFrames);

            for (auto [frameId, aTile] : enumerate(ret.animatedTiles.back())) {
                aTile = getTile(frameId, tileId);
            }
        }
    }

    if (!invalidTiles.empty()) {
        err.addError(std::make_unique<InvalidImageError>(std::move(invalidTiles)));
    }

    return ret;
}

static void buildTilesetAndTilemap(AnimatedTilesetData& aniTileset, const usize& mapSize, const AnimatedTilesetIntermediate& input)
{
    aniTileset.tileMap = grid<Snes::TilemapEntry>(mapSize);
    assert(aniTileset.tileMap.cellCount() == input.tileMap.size());

    {
        Snes::TilesetInserter8px staticTilesetInserter(aniTileset.staticTiles);
        auto tmIt = input.tileMap.begin();
        for (auto& tmEntry : aniTileset.tileMap) {
            const auto& tm = *tmIt++;

            if (tm.isAnimated == false) {
                const auto& tile = input.staticTiles.at(tm.tile);
                auto to = staticTilesetInserter.getOrInsert(tile);

                tmEntry.setCharacter(to.tileId);
                tmEntry.setPalette(tm.palette);
                tmEntry.setHFlip(to.hFlip);
                tmEntry.setVFlip(to.vFlip);
            }
        }
        assert(tmIt == input.tileMap.end());
    }

    assert(aniTileset.animatedTiles.empty());

    if (!input.animatedTiles.empty()) {
        unsigned nAnimatedFrames = input.animatedTiles.front().size();

        aniTileset.animatedTiles.resize(nAnimatedFrames);

        unsigned aniTileOffset = aniTileset.staticTiles.size();

        Snes::AnimatedTilesetInserter<8> aniTilesetInserter(aniTileset.animatedTiles);
        auto tmIt = input.tileMap.begin();
        for (auto& tmEntry : aniTileset.tileMap) {
            const auto& tm = *tmIt++;

            if (tm.isAnimated == true) {
                const auto& tiles = input.animatedTiles.at(tm.tile);
                auto to = aniTilesetInserter.getOrInsert(tiles);

                tmEntry.setCharacter(to.tileId + aniTileOffset);
                tmEntry.setPalette(tm.palette);
                tmEntry.setHFlip(to.hFlip);
                tmEntry.setVFlip(to.vFlip);
            }
        }
        assert(tmIt == input.tileMap.end());
    }
}

bool AnimationFramesInput::isBitDepthValid() const
{
    return bitDepth == 2 || bitDepth == 4 || bitDepth == 8;
}

static bool validate(const AnimationFramesInput& input, ErrorList& err)
{
    bool valid = true;

    if (!input.isBitDepthValid()) {
        err.addErrorString("Invalid bit-depth, expected 2, 4 or 8");
        valid = false;
    }

    if (input.frameImageFilenames.empty()) {
        err.addErrorString("Missing frame image");
        valid = false;
    }

    if (!input.conversionPalette.isValid()) {
        err.addErrorString("Missing conversion palette name");
        valid = false;
    }

    std::vector<usize> imageSizes(input.frameImageFilenames.size());

    for (auto [i, imageFilename] : const_enumerate(input.frameImageFilenames)) {
        const auto& image = ImageCache::loadPngImage(imageFilename);
        imageSizes.at(i) = image->size();

        if (image->empty()) {
            err.addErrorString("Missing frame image: ", image->errorString());
            valid = false;
            continue;
        }

        if (image->size().width % 8 != 0 || image->size().height % 8 != 0) {
            err.addErrorString("image size invalid (height and width must be a multiple of 8): ", imageFilename.string());
            valid = false;
        }
    }

    if (valid) {
        for (const usize& imgSize : imageSizes) {
            if (imgSize != imageSizes.front()) {
                err.addErrorString("All frame images must be the same size");
                valid = false;
                break;
            }
        }
    }

    return valid;
}

static bool validate(const AnimatedTilesetData& input, ErrorList& err)
{
    bool valid = true;

    for (const auto& at : input.animatedTiles) {
        if (at.size() != input.nAnimatedTiles()) {

            err.addErrorString("animatedTiles is invalid");
            valid = false;
        }
    }

    if (input.staticTiles.size() == 0) {
        err.addErrorString("Expected at least one static tile");
        valid = false;
    }

    auto validateMax = [&](unsigned v, unsigned max, const char* msg) {
        if (v > max) {
            err.addErrorString(msg, " (", v, ", max: ", max, ")");
            valid = false;
        }
    };
    validateMax(input.animatedTilesBlockSize(), input.MAX_ANIMATED_TILES_BLOCK_SIZE, "Too many animated tiles");
    validateMax(input.animatedTilesFrameSize(), input.MAX_ANIMATED_TILES_FRAME_SIZE, "Animated frame size too large");
    validateMax(input.staticTiles.size() + input.nAnimatedTiles(), input.MAX_SNES_TILES, "Too many tiles");

    return valid;
}

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

std::optional<AnimatedTilesetData>
convertAnimationFrames(const AnimationFramesInput& input,
                       const Project::DataStore<Resources::PaletteData>& projectDataStore,
                       ErrorList& err)
{
    bool valid = validate(input, err);
    if (!valid) {
        return std::nullopt;
    }

    const unsigned initialErrorCount = err.errorCount();

    const auto& firstImageFilename = input.frameImageFilenames.front();
    const auto& imgSize = ImageCache::loadPngImage(firstImageFilename)->size();

    AnimatedTilesetData ret;
    ret.bitDepth = input.bitDepth;
    ret.animationDelay = input.animationDelay;

    const usize mapSize(imgSize.width / 8, imgSize.height / 8);

    auto paletteIndex = projectDataStore.indexOf(input.conversionPalette);
    auto paletteData = projectDataStore.at(paletteIndex);
    if (!paletteData) {
        err.addErrorString("Cannot find palette: ", input.conversionPalette);
        return std::nullopt;
    }

    ret.conversionPaletteIndex = *paletteIndex;

    const auto frameTiles = tilesFromFrameImages(input, paletteData->conversionPalette, err);
    if (initialErrorCount != err.errorCount()) {
        return std::nullopt;
    }
    const auto tilesetIntermediate = combineFrameTiles(frameTiles, mapSize.width, err);

    if (input.addTransparentTile) {
        ret.staticTiles.emplace_back();
    }

    buildTilesetAndTilemap(ret, mapSize, tilesetIntermediate);

    if (initialErrorCount != err.errorCount()) {
        return std::nullopt;
    }

    valid &= validate(ret, err);

    if (!valid) {
        return std::nullopt;
    }

    return ret;
}

}
