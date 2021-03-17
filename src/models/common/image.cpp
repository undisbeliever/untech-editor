/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "image.h"
#include "stringbuilder.h"
#include "vendor/lodepng/lodepng.h"
#include <cassert>

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
    assert(uintptr_t(_imageData.data()) % sizeof(rgba) == 0);
}

Image::Image(unsigned width, unsigned height)
    : _size(width, height)
    , _imageData(width * height * sizeof(rgba))
    , _errorString()
{
    assert(uintptr_t(_imageData.data()) % alignof(rgba) == 0);
    assert(uintptr_t(_imageData.data()) % sizeof(rgba) == 0);
}

Image::Image(unsigned width, unsigned height, std::vector<unsigned char>&& imageData)
    : _size(width, height)
    , _imageData(imageData)
    , _errorString()
{
    assert(uintptr_t(_imageData.data()) % alignof(rgba) == 0);
    assert(uintptr_t(_imageData.data()) % sizeof(rgba) == 0);

    if (imageData.size() != width * height * sizeof(rgba)) {
        throw std::logic_error("Invalid imageData size");
    }
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

std::shared_ptr<Image> Image::loadPngImage_shared(const std::filesystem::path& filename)
{
    unsigned width;
    unsigned height;
    std::vector<uint8_t> pixels;

    const bool error = lodepng::decode(pixels, width, height, filename.string());

    std::string errorString;

    if (error) {
        width = 0;
        height = 0;
    }

    assert(pixels.size() == width * height * sizeof(rgba));

    auto image = std::make_shared<Image>(width, height, std::move(pixels));

    if (!error) {
        // Ensure all transparent pixels have the same value (`rgba(0, 0, 0, 0)`).
        std::for_each(image->data(), image->data() + image->dataSize(),
                      [](rgba& c) { if (c.alpha == 0) { c = rgba(0, 0, 0, 0); } });
    }
    else {
        image->_errorString = stringBuilder(filename.string(), ": ", lodepng_error_text(error));
    }

    return image;
}
