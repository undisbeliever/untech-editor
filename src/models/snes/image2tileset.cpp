/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "image2tileset.h"
#include "bit-depth.h"
#include "tile-data.h"
#include "models/common/exceptions.h"
#include "models/common/file.h"
#include "models/common/iterators.h"
#include "models/common/stringbuilder.h"
#include "models/snes/convert-snescolor.h"

namespace UnTech::Snes {

ImageToTileset::ImageToTileset(BitDepthSpecial bd)
    : _bitDepth(bd)
    , _tileset()
    , _palette()
{
}

void ImageToTileset::convertAndSave(const IndexedImage& image, BitDepthSpecial bitDepth,
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
    const std::vector<uint8_t> data = snesTileData(_tileset, _bitDepth);
    File::writeFile(filename, data);
}

void ImageToTileset::writePalette(const std::filesystem::path& filename) const
{
    std::vector<uint8_t> out(_palette.size() * 2);
    auto outIt = out.begin();

    for (const auto& c : _palette) {
        *outIt++ = c.data() & 0xFF;
        *outIt++ = c.data() >> 8;
    }
    assert(outIt == out.end());

    File::writeFile(filename, out);
}

void ImageToTileset::process(const IndexedImage& image)
{
    processPalette(image);
    processTileset(image);
}

void ImageToTileset::processPalette(const IndexedImage& image)
{
    const unsigned nColors = colorsForBitDepth(_bitDepth);

    if (image.palette().size() > nColors) {
        throw runtime_error(u8"Too many colors in image, maximum allowed is ", nColors);
    }

    _palette.resize(nColors);

    assert(image.palette().size() <= _palette.size());
    std::transform(image.palette().begin(), image.palette().end(),
                   _palette.begin(),
                   [](const rgba& c) { return Snes::toSnesColor(c); });
}

void ImageToTileset::processTileset(const IndexedImage& image)
{
    constexpr unsigned TILE_SIZE = Tile8px::TILE_SIZE;
    const uint8_t pixelMask = pixelMaskForBitdepth(_bitDepth);

    if (image.size().width % TILE_SIZE != 0
        || image.size().height % TILE_SIZE != 0) {

        throw runtime_error(u8"Image size is not a multiple of ", TILE_SIZE);
    }

    unsigned tileWidth = image.size().width / TILE_SIZE;
    unsigned tileHeight = image.size().height / TILE_SIZE;

    _tileset.resize(tileWidth * tileHeight);
    auto tilesetIt = _tileset.begin();

    for (const auto tileY : range(tileHeight)) {
        for (const auto tileX : range(tileWidth)) {
            Tile8px& tile = *tilesetIt++;
            auto tData = tile.data().begin();

            assert(tileX * TILE_SIZE <= image.size().width && tileY * TILE_SIZE <= image.size().height);

            for (const auto py : range(TILE_SIZE)) {
                const auto imgBits = image.scanline(tileY * TILE_SIZE + py).subspan(tileX * TILE_SIZE, TILE_SIZE);

                for (const auto& c : imgBits) {
                    *tData++ = c & pixelMask;
                }
            }
            assert(tData == tile.data().end());
        }
    }
    assert(tilesetIt == _tileset.end());
}

}
