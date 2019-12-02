/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "image.h"
#include "vendor/lodepng/lodepng.h"
#include <cassert>
#include <sstream>

using namespace UnTech;

static_assert(sizeof(rgba) == 4, "rgba is the wrong size");

Image::Image()
    : _size(0, 0)
    , _imageData()
    , _errorString()
{
}

Image::Image(const usize& size)
    : _size(size)
    , _imageData(size.width * size.height * sizeof(rgba))
    , _errorString()
{
    assert(uintptr_t(_imageData.data()) % alignof(rgba) == 0);
}

Image::Image(unsigned width, unsigned height)
    : _size(width, height)
    , _imageData(width * height * sizeof(rgba))
    , _errorString()
{
    assert(uintptr_t(_imageData.data()) % alignof(rgba) == 0);
}

void Image::erase()
{
    _size = usize(0, 0);
    _imageData.clear();
}

void Image::fill(const rgba& color)
{
    if (empty()) {
        return;
    }

    auto* ptr = data();
    auto* ptrEnd = data() + _size.height * _size.width;

    while (ptr < ptrEnd) {
        *ptr++ = color;
    }
}

bool Image::loadPngImage(const std::filesystem::path& filename)
{
    erase();
    auto error = lodepng::decode(_imageData, _size.width, _size.height, filename);

    if (error) {
        erase();

        std::stringstream msg;
        msg << filename << ": " << lodepng_error_text(error);
        _errorString = msg.str();

        return false;
    }

    assert(_imageData.size() == _size.width * _size.height * sizeof(rgba));
    assert(uintptr_t(_imageData.data()) % alignof(rgba) == 0);

    if (not empty()) {
        // Ensure all transparent pixels have the same value (`rgba(0, 0, 0, 0)`).
        std::for_each(data(), data() + dataSize(),
                      [](rgba& c) { if (c.alpha == 0) { c = rgba(0, 0, 0, 0); } });
    }

    return true;
}
