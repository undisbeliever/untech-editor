/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "background-image.h"
#include "models/common/bytevectorhelper.h"
#include "models/common/errorlist.h"
#include "models/common/imagecache.h"
#include "models/lz4/lz4.h"
#include "models/project/project-data.h"
#include "models/snes/bit-depth.h"
#include "models/snes/tilesetinserter.h"
#include <algorithm>
#include <cassert>

#include "tile-extractor.hpp"

namespace UnTech::Resources {

static constexpr unsigned HEADER_DATA_SIZE = 1;

static constexpr unsigned BG_MAP_WIDTH = 32;
static constexpr unsigned BG_MAP_HEIGHT = 32;
static constexpr unsigned BG_MAP_DATA_SIZE = BG_MAP_WIDTH * BG_MAP_HEIGHT * 2;

static bool validate(const BackgroundImageData& input, ErrorList& err);

static bool validate(const BackgroundImageInput& input, ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };

    if (input.name.isValid() == false) {
        addError(u8"Expected name");
    }

    const auto& image = ImageCache::loadPngImage(input.imageFilename);
    if (image->empty()) {
        addError(u8"Missing frame image: ", image->errorString());
    }

    if (!image->empty()) {
        if (image->size().width % 8 != 0 || image->size().height % 8 != 0) {
            addError(u8"image size invalid (height and width must be a multiple of 8): ", input.imageFilename.u8string());
        }

        if (image->size().width > 512 || image->size().height > 512) {
            addError(u8"image is too large (maximum size is 512x512px)");
        }
    }

    if (input.bitDepth > Snes::BitDepth::BD_4BPP) {
        if (input.firstPalette != 0) {
            addError(u8"When bitDepth is > 4bpp, firstPalette must be 0");
        }
        if (input.nPalettes != 1) {
            addError(u8"When bitDepth is > 4bpp, nPalettes must be 1");
        }
    }

    if (input.nPalettes < 1 || input.nPalettes > 8) {
        addError(u8"nPalettes out of range (1-8)");
    }

    return valid;
}

std::shared_ptr<const BackgroundImageData>
convertBackgroundImage(const BackgroundImageInput& input, const Project::DataStore<PaletteData>& projectDataStore,
                       ErrorList& err)
{
    bool valid = validate(input, err);
    if (!valid) {
        return nullptr;
    }

    const auto& image = ImageCache::loadPngImage(input.imageFilename);
    assert(image);

    const auto paletteIndexAndData = projectDataStore.indexAndDataFor(input.conversionPlette);
    if (!paletteIndexAndData) {
        err.addErrorString(u8"Cannot find palette: ", input.conversionPlette);
        return nullptr;
    }
    const auto& palette = paletteIndexAndData->second->conversionPalette;

    const unsigned lastColor = (input.firstPalette + input.nPalettes) * Snes::colorsForBitDepth(input.bitDepth);
    if (lastColor > palette.size()) {
        err.addErrorString(u8"Palette ", input.conversionPlette, u8" does not contain enough colors (expected ", lastColor, u8")");
        return nullptr;
    }

    std::vector<InvalidImageTile> invalidTiles;
    std::vector<TileAndPalette> extractedTiles = tilesFromImage(*image, input.bitDepth,
                                                                palette, input.firstPalette, input.nPalettes,
                                                                invalidTiles);
    if (!invalidTiles.empty()) {
        err.addError(std::make_unique<InvalidImageError>(std::move(invalidTiles)));
        return nullptr;
    }

    auto ret = std::make_shared<BackgroundImageData>();

    ret->bitDepth = input.bitDepth;
    ret->conversionPaletteIndex = paletteIndexAndData->first;

    Snes::TilesetInserter8px staticTilesetInserter(ret->tiles);

    const usize mapSize(image->size().width / 8, image->size().height / 8);
    ret->tileMap = grid<Snes::TilemapEntry>(mapSize);

    auto tmIt = ret->tileMap.begin();
    for (auto& ex : extractedTiles) {
        auto& tm = *tmIt++;

        const auto to = staticTilesetInserter.getOrInsert(ex.tile);

        tm.setCharacter(to.tileId);
        tm.setPalette(ex.palette & 7);
        tm.setOrder(input.defaultOrder);
        tm.setHFlip(to.hFlip);
        tm.setVFlip(to.vFlip);
    }
    assert(tmIt == ret->tileMap.end());

    valid &= validate(*ret, err);

    if (!valid) {
        ret = nullptr;
    }

    return ret;
}

static bool validate(const BackgroundImageData& input, ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };

    if (input.tiles.empty()) {
        addError(u8"Expected at least one tile");
    }
    if (input.tiles.size() > input.MAX_SNES_TILES) {
        addError(u8"Too many tiles in tileset (", input.tiles.size(), u8", max: ", input.MAX_SNES_TILES, u8")");
    }

    if (input.tileMap.empty() || input.nTilemaps() == 0) {
        addError(u8"tileMap empty");
    }
    if (input.tileMap.width() > 64 || input.tileMap.height() > 64) {
        addError(u8"tileMap too large");
    }

    if (input.nTilemaps() >= input.MAX_N_TILEMAPS) {
        addError(u8"Too many tileMaps (", input.nTilemaps(), u8", max: ", input.MAX_N_TILEMAPS, u8")");
    }

    if (input.uncompressedDataSize() > input.MAX_UNCOMPRESSED_DATA_SIZE) {
        addError(u8"data too large (", input.uncompressedDataSize(), u8" bytes, max: ", input.MAX_UNCOMPRESSED_DATA_SIZE, u8")");
    }

    return valid;
}

unsigned BackgroundImageData::nTilemaps() const
{
    return ((tileMap.width() - 1) / BG_MAP_WIDTH + 1)
           * ((tileMap.height() - 1) / BG_MAP_HEIGHT + 1);
}

unsigned BackgroundImageData::uncompressedDataSize() const
{
    return tilemapDataSize() + tilesetDataSize();
}

unsigned BackgroundImageData::tilesetDataSize() const
{
    return tiles.size() * Snes::snesTileSizeForBitdepth(bitDepth);
}

unsigned BackgroundImageData::tilemapDataSize() const
{
    return nTilemaps() * BG_MAP_DATA_SIZE;
}

static void convertBackgroundMap(std::vector<uint8_t>& out, const grid<Snes::TilemapEntry>& grid, unsigned xOffset, unsigned yOffset)
{
    const size_t startIndex = out.size();
    out.resize(out.size() + BG_MAP_DATA_SIZE, 0);

    const unsigned endX = std::min<unsigned>(BG_MAP_WIDTH, grid.width() - xOffset);
    const unsigned endY = std::min<unsigned>(BG_MAP_HEIGHT, grid.height() - yOffset);

    auto it = out.begin() + startIndex;
    for (const auto y : range(endY)) {
        for (const auto x : range(endX)) {
            const auto& tm = grid.at(x + xOffset, y + yOffset);

            *it++ = tm.data & 0xff;
            *it++ = (tm.data >> 8) & 0xff;
        }
    }
    assert(it == out.end());
}

const int BackgroundImageData::BACKGROUND_IMAGE_FORMAT_VERSION = 1;

std::vector<uint8_t> BackgroundImageData::exportSnesData() const
{
    // ::TODO support extended maps::
    assert(tileMap.width() <= 64);
    assert(tileMap.height() <= 64);

    const unsigned dataSize = uncompressedDataSize();
    assert(dataSize < MAX_UNCOMPRESSED_DATA_SIZE);

    std::vector<uint8_t> mapAndTileData;
    mapAndTileData.reserve(dataSize);

    // tileMaps
    for (unsigned y = 0; y < tileMap.height(); y += 32) {
        for (unsigned x = 0; x < tileMap.height(); x += 32) {
            convertBackgroundMap(mapAndTileData, tileMap, x, y);
        }
    }

    // tileData
    std::vector<uint8_t> tileData = Snes::snesTileData(tiles, bitDepth);
    mapAndTileData.insert(mapAndTileData.end(), tileData.begin(), tileData.end());

    assert(mapAndTileData.size() == dataSize);
    const auto compressedData = lz4HcCompress(mapAndTileData);

    // engine data
    std::vector<uint8_t> data;
    data.reserve(compressedData.size() + HEADER_DATA_SIZE);

    // header
    assert(nTilemaps() > 0);
    const unsigned tilemapCount = nTilemaps() - 1;
    assert((tilemapCount & 0xf) == tilemapCount);
    data.emplace_back(tilemapCount);

    data.insert(data.end(), compressedData.begin(), compressedData.end());

    return data;
}

}
