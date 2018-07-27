/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animation-frames-input.h"
#include "resources.h"
#include "models/common/bytevectorhelper.h"
#include "models/common/imagecache.h"
#include "models/lz4/lz4.h"
#include "models/snes/animatedtilesetinserter.h"
#include "models/snes/tilesetinserter.h"
#include <algorithm>
#include <cassert>

namespace UnTech {
namespace Resources {

using Tile8px = Snes::Tile8px;

struct FrameTile {
    Tile8px tile;
    unsigned palette;
};

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

static bool extractTile8px(Tile8px& tile, const rgba* imgBits, unsigned pixelsPerScanline,
                           const std::vector<Snes::SnesColor>::const_iterator pBegin,
                           const std::vector<Snes::SnesColor>::const_iterator pEnd)
{
    const static unsigned TS = tile.TILE_SIZE;

    for (unsigned y = 0; y < TS; y++) {
        const rgba* scanline = imgBits + y * pixelsPerScanline;

        for (unsigned x = 0; x < TS; x++) {
            const Snes::SnesColor c(scanline[x]);

            auto pIt = std::find(pBegin, pEnd, c);
            if (pIt == pEnd) {
                // color not found in palette
                return false;
            }

            tile.setPixel(x, y, std::distance(pBegin, pIt));
        }
    }

    return true;
}

static bool extractFrameTile(FrameTile& ft, const Image& image, const unsigned x, const unsigned y,
                             const std::vector<Snes::SnesColor>& palettes, unsigned colorsPerPalette)
{
    const unsigned pps = image.pixelsPerScanline();
    const rgba* imgBits = image.scanline(y) + x;

    for (size_t pIndex = 0, pId = 0; pIndex < palettes.size(); pIndex += colorsPerPalette, pId++) {
        auto pStart = palettes.begin() + pIndex;
        auto pEnd = pIndex + colorsPerPalette < palettes.size() ? pStart + colorsPerPalette
                                                                : palettes.end();

        bool s = extractTile8px(ft.tile, imgBits, pps, pStart, pEnd);

        if (s) {
            ft.palette = pId;
            return true;
        }
    }

    return false;
}

static std::vector<FrameTile> tilesFromFrameImage(const Image& image, unsigned frameId,
                                                  const std::vector<Snes::SnesColor>& palettes, unsigned colorsPerPalette,
                                                  ErrorList& err)
{
    const static unsigned TS = 8;

    const usize iSize = image.size();

    unsigned tw = iSize.width / TS;
    unsigned th = iSize.height / TS;
    std::vector<FrameTile> tiles(tw * th);
    auto tileIt = tiles.begin();

    for (unsigned y = 0; y < iSize.height; y += TS) {
        for (unsigned x = 0; x < iSize.width; x += TS) {
            bool s = extractFrameTile(*tileIt, image, x, y, palettes, colorsPerPalette);
            if (!s) {
                err.addInvalidImageTile(frameId, TS, x, y, ErrorList::NO_PALETTE_FOUND);
            }
            tileIt++;
        }
    }
    assert(tileIt == tiles.end());

    return tiles;
}

static std::vector<std::vector<FrameTile>> tilesFromFrameImages(const AnimationFramesInput input,
                                                                const std::vector<Snes::SnesColor>& palettes,
                                                                ErrorList& err)
{
    const unsigned colorsPerPalette = 1 << input.bitDepth;

    std::vector<std::vector<FrameTile>> frameTiles;
    frameTiles.reserve(input.frameImageFilenames.size());

    const unsigned nFrames = input.frameImageFilenames.size();
    for (unsigned frameId = 0; frameId < nFrames; frameId++) {
        const auto& image = ImageCache::loadPngImage(input.frameImageFilenames.at(frameId));
        frameTiles.emplace_back(
            tilesFromFrameImage(*image, frameId, palettes, colorsPerPalette, err));
    }

    return frameTiles;
}

static AnimatedTilesetIntermediate combineFrameTiles(const std::vector<std::vector<FrameTile>>& frameTiles, unsigned tileWidth,
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

    AnimatedTilesetIntermediate ret;
    ret.animatedTiles.reserve(64);
    ret.tileMap.resize(nTiles);

    for (unsigned tileId = 0; tileId < nTiles; tileId++) {
        const static unsigned TS = Tile8px::TILE_SIZE;

        auto& tm = ret.tileMap.at(tileId);

        unsigned palette = getPalette(0, tileId);
        for (unsigned frameId = 1; frameId < nFrames; frameId++) {
            if (getPalette(frameId, tileId) != palette) {
                err.addInvalidImageTile(TS, (tileId % tileWidth) * TS, (tileId / tileWidth) * TS, ErrorList::NOT_SAME_PALETTE);
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

    return ret;
}

static void buildTilesetAndTilemap(AnimatedTilesetData& aniTileset, const usize& mapSize, const AnimatedTilesetIntermediate& input)
{
    aniTileset.tileMap = grid<Snes::TilemapEntry>(mapSize);
    assert(aniTileset.tileMap.gridSize() == input.tileMap.size());

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
        err.addError("Invalid bit-depth, expected 2, 4 or 8");
        valid = false;
    }

    if (frameImageFilenames.empty()) {
        err.addError("Missing frame image");
        valid = false;
    }

    std::vector<usize> imageSizes(frameImageFilenames.size());

    for (unsigned i = 0; i < frameImageFilenames.size(); i++) {
        const auto& imageFilename = frameImageFilenames.at(i);
        const auto& image = ImageCache::loadPngImage(imageFilename);
        imageSizes.at(i) = image->size();

        if (image->empty()) {
            err.addError("Missing frame image: " + image->errorString());
            valid = false;
            break;
        }

        if (image->size().width % 8 != 0 || image->size().height % 8 != 0) {
            err.addError("image size invalid (height and width must be a multiple of 8): " + imageFilename);
            valid = false;
        }
    }

    if (valid) {
        for (const usize& imgSize : imageSizes) {
            if (imgSize != imageSizes.front()) {
                err.addError("All frame images must be the same size");
                valid = false;
                break;
            }
        }
    }

    return valid;
}

std::unique_ptr<AnimatedTilesetData>
convertAnimationFrames(const AnimationFramesInput& input, const PaletteInput& paletteInput,
                       ErrorList& err)
{
    bool valid = input.validate(err);
    if (!valid) {
        return nullptr;
    }

    const unsigned initialErrorCount = err.errorCount();

    const auto& firstImageFilename = input.frameImageFilenames.front();
    const auto& imgSize = ImageCache::loadPngImage(firstImageFilename)->size();

    auto ret = std::make_unique<AnimatedTilesetData>(input.bitDepth);
    ret->animationDelay = input.animationDelay;

    const usize mapSize(imgSize.width / 8, imgSize.height / 8);

    const auto palette = extractFirstPalette(paletteInput, input.bitDepth, err);
    if (palette.empty()) {
        return nullptr;
    }

    const auto frameTiles = tilesFromFrameImages(input, palette, err);
    if (initialErrorCount != err.errorCount()) {
        return nullptr;
    }
    const auto tilesetIntermediate = combineFrameTiles(frameTiles, mapSize.width, err);

    if (input.addTransparentTile) {
        ret->staticTiles.addTile();
    }

    buildTilesetAndTilemap(*ret, mapSize, tilesetIntermediate);

    if (initialErrorCount != err.errorCount()) {
        return nullptr;
    }

    valid = ret->validate(err);
    if (!valid) {
        return nullptr;
    }

    return ret;
}
}
}
