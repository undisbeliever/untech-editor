/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "indexedimage.h"
#include "vendor/lodepng/lodepng.h"
#include <cassert>
#include <stdexcept>

using namespace UnTech;

static_assert(sizeof(rgba) == 4, "rgba is the wrong size");

IndexedImage::IndexedImage()
    : _size(0, 0)
    , _imageData()
    , _palette()
    , _errorString()
{
}

IndexedImage::IndexedImage(const usize& size)
    : _size(size)
    , _imageData(size.width * size.height * sizeof(rgba))
    , _palette()
    , _errorString()
{
}

IndexedImage::IndexedImage(unsigned width, unsigned height)
    : _size(width, height)
    , _imageData(width * height * sizeof(rgba))
    , _palette()
    , _errorString()
{
}

void IndexedImage::erase()
{
    _size = usize(0, 0);
    _imageData.clear();
    _palette.clear();
}

void IndexedImage::fill(uint8_t color)
{
    if (color > _palette.size()) {
        color = 0;
    }
    std::fill(_imageData.begin(), _imageData.end(), color);
}

bool IndexedImage::loadPngImage(const std::string& filename)
{
    std::vector<uint8_t> pngFile;
    lodepng::State state;

    state.decoder.color_convert = true;
    state.info_raw.colortype = LodePNGColorType::LCT_PALETTE;
    state.info_raw.bitdepth = 8;

    unsigned error = lodepng::load_file(pngFile, filename);
    if (!error) {
        _imageData.clear();
        lodepng::decode(_imageData, _size.width, _size.height, state, pngFile);
    }

    if (error) {
        erase();

        _errorString = filename + ": " + lodepng_error_text(error);
        return false;
    }

    if (state.info_png.color.colortype != LodePNGColorType::LCT_PALETTE) {
        erase();
        _errorString = filename + ": Not a indexed png file";
        return false;
    }

    if (_imageData.size() != _size.width * _size.height) {
        throw std::runtime_error("invalid lodepng image decoding");
    }

    const rgba* pngPal = reinterpret_cast<rgba*>(state.info_png.color.palette);
    const unsigned pngPalSize = state.info_png.color.palettesize;

    _palette.clear();
    _palette.insert(_palette.begin(), pngPal, pngPal + pngPalSize);

    return true;
}
