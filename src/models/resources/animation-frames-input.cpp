/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animation-frames-input.h"
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

    bool extractedTile = false;

    unsigned pId = 0;
    for (auto palIt = palettes.begin(); palIt < palettes.end(); palIt += colorsPerPalette, pId++) {
        bool s = extractTile8px(ft.tile, imgBits, pps, palIt, palIt + colorsPerPalette);

        if (s) {
            ft.palette = pId;

            extractedTile = true;
            break;
        }
    }

    return extractedTile;
}

static std::vector<FrameTile> tilesFromFrameImage(const Image& image, const std::string& imageFilename,
                                                  const std::vector<Snes::SnesColor>& palettes, unsigned colorsPerPalette)
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
                throw std::runtime_error("Unable to extract tile at position "
                                         + std::to_string(x) + ", " + std::to_string(y)
                                         + ": " + imageFilename);
            }
            tileIt++;
        }
    }
    assert(tileIt == tiles.end());

    return tiles;
}

static std::vector<std::vector<FrameTile>> tilesFromFrameImages(const AnimationFramesInput input,
                                                                const std::vector<Snes::SnesColor>& palettes)
{
    const unsigned colorsPerPalette = 1 << input.bitDepth;

    std::vector<std::vector<FrameTile>> frameTiles;
    frameTiles.reserve(input.frameImages.size());

    for (unsigned i = 0; i < input.frameImages.size(); i++) {
        const auto& image = input.frameImages.at(i);
        const auto& imageFilename = input.frameImageFilenames.at(i);

        frameTiles.emplace_back(
            tilesFromFrameImage(image, imageFilename, palettes, colorsPerPalette));
    }

    return frameTiles;
}

static AnimatedTilesetIntermediate combineFrameTiles(const std::vector<std::vector<FrameTile>>& frameTiles,
                                                     unsigned tileWidth)
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
        auto& tm = ret.tileMap.at(tileId);

        unsigned palette = getPalette(0, tileId);
        for (unsigned frameId = 1; frameId < nFrames; frameId++) {
            if (getPalette(frameId, tileId) != palette) {
                throw std::runtime_error("Tile " + std::to_string(tileId % tileWidth)
                                         + ", " + std::to_string(tileId / tileWidth)
                                         + " must use the same palette in each frame.");
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

static void buildTilesetAndTilemap(AnimatedTilesetData& aniTileset, const AnimatedTilesetIntermediate& input)
{
    auto& tileMap = aniTileset.tileMap;
    tileMap.resize(input.tileMap.size());

    Snes::TilesetInserter8px staticTilesetInserter(aniTileset.staticTiles);
    for (unsigned t = 0; t < tileMap.size(); t++) {
        const auto& tm = input.tileMap.at(t);
        auto& tmEntry = tileMap.at(t);

        if (tm.isAnimated == false) {
            const auto& tile = input.staticTiles.at(tm.tile);
            auto to = staticTilesetInserter.getOrInsert(tile);

            tmEntry.setCharacter(to.tileId);
            tmEntry.setPalette(tm.palette);
            tmEntry.setHFlip(to.hFlip);
            tmEntry.setVFlip(to.vFlip);
        }
    }

    if (!input.animatedTiles.empty()) {
        unsigned nAnimatedFrames = input.animatedTiles.front().size();
        for (unsigned i = 0; i < nAnimatedFrames; i++) {
            aniTileset.animatedTiles.emplace_back(aniTileset.staticTiles.bitDepth());
        }

        unsigned aniTileOffset = aniTileset.staticTiles.size();

        Snes::AnimatedTilesetInserter<Snes::Tileset8px> aniTilesetInserter(aniTileset.animatedTiles);
        for (unsigned t = 0; t < tileMap.size(); t++) {
            const auto& tm = input.tileMap.at(t);
            auto& tmEntry = tileMap.at(t);

            if (tm.isAnimated == true) {
                const auto& tiles = input.animatedTiles.at(tm.tile);
                auto to = aniTilesetInserter.getOrInsert(tiles);

                tmEntry.setCharacter(to.tileId + aniTileOffset);
                tmEntry.setPalette(tm.palette);
                tmEntry.setHFlip(to.hFlip);
                tmEntry.setVFlip(to.vFlip);
            }
        }
    }
}

void AnimationFramesInput::validate() const
{
    if (bitDepth != 2 && bitDepth != 4 && bitDepth != 8) {
        throw std::runtime_error("Invalid bit-depth, expected 2, 4 or 8");
    }

    if (frameImages.empty()) {
        throw std::runtime_error("Missing frame image");
    }

    if (frameImages.size() != frameImageFilenames.size()) {
        throw std::runtime_error("Invalid number of frameImages");
    }

    usize frameSize = frameImages.front().size();

    assert(frameImages.size() == frameImageFilenames.size());
    for (unsigned i = 0; i < frameImages.size(); i++) {
        const auto& image = frameImages.at(i);
        const auto& imageFilename = frameImageFilenames.at(i);

        if (image.empty()) {
            throw std::runtime_error("Missing frame image: " + imageFilename + ": " + image.errorString());
        }

        if (image.size().width % 8 != 0 || image.size().height % 8 != 0) {
            throw std::runtime_error("image size invalid (height and width must be a multiple of 8): " + imageFilename);
        }

        if (image.size() != frameSize) {
            throw std::runtime_error("All frame images must be the same size");
        }
    }
}

AnimatedTilesetData convertAnimationFrames(const AnimationFramesInput& input,
                                           const PaletteInput& paletteInput)
{
    input.validate();

    AnimatedTilesetData ret(input.bitDepth);

    ret.animationDelay = input.animationDelay;
    ret.mapWidth = input.frameImages.front().size().width / 8;
    ret.mapHeight = input.frameImages.front().size().height / 8;

    const auto palettes = extractFirstPalette(paletteInput, input.bitDepth);

    const auto frameTiles = tilesFromFrameImages(input, palettes);
    const auto tilesetIntermediate = combineFrameTiles(frameTiles, ret.mapWidth);

    if (input.addTransparentTile) {
        ret.staticTiles.addTile();
    }

    buildTilesetAndTilemap(ret, tilesetIntermediate);

    return ret;
}
}
}
