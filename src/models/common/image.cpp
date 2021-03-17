/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "image.h"
#include "stringbuilder.h"
#include "vendor/lodepng/lodepng.h"

#ifndef LODEPNG_COMPILE_ALLOCATORS
#error "Cannot use custom lodepng allocators"
#endif

using namespace UnTech;

static_assert(sizeof(rgba) == 4, "rgba is the wrong size");

Image::Image(const usize size)
    : _size(size)
    , _errorString()
    , _imageData(reinterpret_cast<rgba*>(malloc(size.width * size.height * sizeof(rgba))))
    , _imageDataEnd(_imageData + (size.width * size.height))
{
    assert(_size.width > 0 && _size.height > 0);

    if (_imageData == nullptr) {
        throw std::bad_alloc();
    }

    assert(uintptr_t(_imageData) % alignof(rgba) == 0);
    assert(uintptr_t(_imageData) % sizeof(rgba) == 0);
}

Image::Image(const usize size, rgba*&& data, Image::PrivateToken)
    : _size(size)
    , _errorString()
    , _imageData(data)
    , _imageDataEnd(_imageData + (size.width * size.height))
{
    assert(_size.width > 0 && _size.height > 0);

    if (_imageData == nullptr) {
        throw std::bad_alloc();
    }

    assert(uintptr_t(_imageData) % alignof(rgba) == 0);
    assert(uintptr_t(_imageData) % sizeof(rgba) == 0);
}

Image::Image(std::string&& errorString, Image::PrivateToken)
    : _size(0, 0)
    , _errorString(std::move(errorString))
    , _imageData(nullptr)
    , _imageDataEnd(nullptr)
{
}

Image::~Image()
{
    if (_imageData) {
        // Cannot use delete[] here, `lodepng_decode32_file` allocates memory using `malloc`.
        free(_imageData);
    }
}

void Image::fill(const rgba& color)
{
    if (empty()) {
        return;
    }

    std::fill(_imageData, _imageDataEnd, color);
}

std::shared_ptr<Image> Image::loadPngImage_shared(const std::filesystem::path& filename)
{
    usize size;
    rgba* pixels = nullptr;

    const auto error = lodepng_decode32_file(reinterpret_cast<uint8_t**>(&pixels),
                                             &size.width, &size.height, filename.string().c_str());

    if (!error) {
        auto image = std::make_shared<Image>(size, std::move(pixels), PrivateToken{});

        std::for_each(image->data(), image->dataEnd(),
                      [](rgba& c) { if (c.alpha == 0) { c = rgba(0, 0, 0, 0); } });

        return image;
    }
    else {
        if (pixels) {
            free(pixels);
        }

        return invalidImageWithErrorMessage(stringBuilder(filename.string(), ": ", lodepng_error_text(error)));
    }
}

std::shared_ptr<Image> Image::invalidImageWithErrorMessage(std::string&& error)
{
    return std::make_shared<Image>(std::move(error), PrivateToken{});
}
