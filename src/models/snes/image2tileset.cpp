/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "image2tileset.h"
#include "models/common/file.h"
#include "models/common/stringbuilder.h"
#include <stdexcept>

using namespace UnTech;
using namespace UnTech::Snes;

ImageToTileset::ImageToTileset(int bitDepth)
    : _tileset(bitDepth)
    , _palette()
{
}

void ImageToTileset::convertAndSave(const IndexedImage& image, int bitDepth,
                                    const std::filesystem::path& tilesetFile, const std::filesystem::path& paletteFile)
{
    ImageToTileset converter(bitDepth);
    converter.process(image);

    if (!tilesetFile.empty()) {
        converter.writeTileset(tilesetFile);
    }
    if (!paletteFile.empty()) {
        converter.writePalette(paletteFile);
    }
}

void ImageToTileset::writeTileset(const std::filesystem::path& filename) const
{
    std::vector<uint8_t> data = _tileset.snesData();
    File::atomicWrite(filename, data);
}

void ImageToTileset::writePalette(const std::filesystem::path& filename) const
{
    std::vector<uint8_t> data(_palette.size() * 2);
    auto* ptr = data.data();

    for (const auto& c : _palette) {
        *ptr++ = c.data() & 0xFF;
        *ptr++ = c.data() >> 8;
    }

    assert(ptr == data.data() + data.size());

    File::atomicWrite(filename, data);
}

void ImageToTileset::process(const IndexedImage& image)
{
    processPalette(image);
    processTileset(image);
}

void ImageToTileset::processPalette(const IndexedImage& image)
{
    const unsigned nColors = _tileset.colorsPerTile();

    if (image.palette().size() > nColors) {
        throw std::runtime_error(stringBuilder("Too many colors in image, maximum allowed is ", nColors));
    }

    _palette.resize(nColors);

    assert(image.palette().size() <= _palette.size());
    std::transform(image.palette().begin(), image.palette().end(),
                   _palette.begin(),
                   [](const rgba& c) { return SnesColor(c); });
}

void ImageToTileset::processTileset(const IndexedImage& image)
{
    constexpr unsigned TILE_SIZE = Tile8px::TILE_SIZE;
    const uint8_t pixelMask = _tileset.pixelMask();

    if (image.size().width % TILE_SIZE != 0
        || image.size().height % TILE_SIZE != 0) {

        throw std::runtime_error(stringBuilder("Image size is not a multiple of ", TILE_SIZE));
    }

    unsigned tileWidth = image.size().width / TILE_SIZE;
    unsigned tileHeight = image.size().height / TILE_SIZE;

    for (unsigned tileY = 0; tileY < tileHeight; tileY++) {
        for (unsigned tileX = 0; tileX < tileWidth; tileX++) {
            Tile8px tile;
            uint8_t* tData = tile.rawData();

            for (unsigned py = 0; py < TILE_SIZE; py++) {
                const uint8_t* imgData = image.scanline(tileY * TILE_SIZE + py) + tileX * TILE_SIZE;

                for (unsigned px = 0; px < TILE_SIZE; px++) {
                    *tData++ = *imgData++ & pixelMask;
                }
            }

            _tileset.addTile(tile);
        }
    }
}
