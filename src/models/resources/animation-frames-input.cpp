/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animation-frames-input.h"
#include "invalid-image-error.h"
#include "models/common/bytevectorhelper.h"
#include "models/common/errorlist.h"
#include "models/common/imagecache.h"
#include "models/lz4/lz4.h"
#include "models/project/project-data.h"
#include "models/snes/animatedtilesetinserter.h"
#include "models/snes/tilesetinserter.h"
#include <algorithm>
#include <cassert>

#include "tile-extractor.hpp"

namespace UnTech {
namespace Resources {

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

    const unsigned nFrames = input.frameImageFilenames.size();
    for (unsigned frameId = 0; frameId < nFrames; frameId++) {
        auto imageErr = std::make_unique<InvalidImageError>(frameId);

        const auto& image = ImageCache::loadPngImage(input.frameImageFilenames.at(frameId));
        frameTiles.emplace_back(
            tilesFromImage(*image, input.bitDepth, palette, 0, 8, *imageErr));

        if (imageErr->hasError()) {
            err.addError(std::move(imageErr));
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

    auto imageErr = std::make_unique<InvalidImageError>();

    AnimatedTilesetIntermediate ret;
    ret.animatedTiles.reserve(64);
    ret.tileMap.resize(nTiles);

    for (unsigned tileId = 0; tileId < nTiles; tileId++) {
        const static unsigned TS = Tile8px::TILE_SIZE;

        auto& tm = ret.tileMap.at(tileId);

        unsigned palette = getPalette(0, tileId);
        for (unsigned frameId = 1; frameId < nFrames; frameId++) {
            if (getPalette(frameId, tileId) != palette) {
                imageErr->addInvalidTile(TS, (tileId % tileWidth) * TS, (tileId / tileWidth) * TS, InvalidImageError::NOT_SAME_PALETTE);
                break;
            }
        }
        tm.palette = palette;

        const Tile8px& firstTile = getTile(0, tileId);
        bool isAnimated = false;
        for (unsigned frameId = 1; frameId < nFrames; frameId++) {
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
            auto& animatedTile = ret.animatedTiles.back();
            for (unsigned frameId = 0; frameId < nFrames; frameId++) {
                animatedTile.at(frameId) = getTile(frameId, tileId);
            }
        }
    }

    if (imageErr->hasError()) {
        err.addError(std::move(imageErr));
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

    if (!input.animatedTiles.empty()) {
        unsigned nAnimatedFrames = input.animatedTiles.front().size();
        for (unsigned i = 0; i < nAnimatedFrames; i++) {
            aniTileset.animatedTiles.emplace_back(aniTileset.staticTiles.bitDepth());
        }

        unsigned aniTileOffset = aniTileset.staticTiles.size();

        Snes::AnimatedTilesetInserter<Snes::Tileset8px> aniTilesetInserter(aniTileset.animatedTiles);
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

bool AnimationFramesInput::validate(ErrorList& err) const
{
    bool valid = true;

    if (bitDepth != 2 && bitDepth != 4 && bitDepth != 8) {
        err.addErrorString("Invalid bit-depth, expected 2, 4 or 8");
        valid = false;
    }

    if (frameImageFilenames.empty()) {
        err.addErrorString("Missing frame image");
        valid = false;
    }

    if (!conversionPalette.isValid()) {
        err.addErrorString("Missing conversion palette name");
        valid = false;
    }

    std::vector<usize> imageSizes(frameImageFilenames.size());

    for (unsigned i = 0; i < frameImageFilenames.size(); i++) {
        const auto& imageFilename = frameImageFilenames.at(i);
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

std::optional<AnimatedTilesetData>
convertAnimationFrames(const AnimationFramesInput& input,
                       const Project::DataStore<Resources::PaletteData>& projectDataStore,
                       ErrorList& err)
{
    bool valid = input.validate(err);
    if (!valid) {
        return std::nullopt;
    }

    const unsigned initialErrorCount = err.errorCount();

    const auto& firstImageFilename = input.frameImageFilenames.front();
    const auto& imgSize = ImageCache::loadPngImage(firstImageFilename)->size();

    AnimatedTilesetData ret(input.bitDepth);
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
        ret.staticTiles.addTile();
    }

    buildTilesetAndTilemap(ret, mapSize, tilesetIntermediate);

    if (initialErrorCount != err.errorCount()) {
        return std::nullopt;
    }

    valid = ret.validate(err);
    if (!valid) {
        return std::nullopt;
    }

    return ret;
}

}
}
