/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "aabb.h"
#include "rgba.h"
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace UnTech {

/**
 * A simple image container class that contains a 32bpp RGBA image.
 */
class Image {
public:
    ~Image() = default;
    Image(const Image&) = default;
    Image(Image&&) = default;
    Image& operator=(const Image&) = default;
    Image& operator=(Image&&) = default;

    Image();
    Image(const usize& size);
    Image(unsigned width, unsigned height);

    usize size() const { return _size; }
    std::string errorString() const { return _errorString; }

    void erase();

    void fill(const rgba& color);

    /**
     * Loads a PNG image from a filename.
     *
     * This overrides the current image.
     *
     * NOTE: This method will transform all pixels with an alpha value to 0 to
     *       `rgba(0, 0, 0, 0)`.
     *
     * If the image cannot be loaded then:
     *   - return false
     *   - the image is erased.
     *   _ errorString is set.
     */
    bool loadPngImage(const std::filesystem::path& filename);

    /**
     * Returns true if the image is empty.
     */
    bool empty() const { return _size.width == 0 || _size.height == 0; }

    inline unsigned pixelsPerScanline() const
    {
        return _size.width;
    }

    inline size_t dataSize() const
    {
        return _size.width * _size.height;
    }

    inline rgba* data()
    {
        return reinterpret_cast<rgba*>(_imageData.data());
    }

    inline rgba* scanline(unsigned y)
    {
        if (y >= _size.height) {
            throw std::out_of_range("Image::scanline out of range");
        }
        return data() + (y * pixelsPerScanline());
    }

    inline const rgba* data() const
    {
        return reinterpret_cast<const rgba*>(_imageData.data());
    }

    inline const rgba* scanline(unsigned y) const
    {
        if (y >= _size.height) {
            throw std::out_of_range("Image::scanline out of range");
        }
        return data() + (y * pixelsPerScanline());
    }

    inline rgba getPixel(unsigned x, unsigned y) const
    {
        if (x >= _size.width || y >= _size.height) {
            throw std::out_of_range("Image::getPixel out of range");
        }
        return *(data() + x + (y * pixelsPerScanline()));
    }

private:
    usize _size;
    std::vector<unsigned char> _imageData;
    std::string _errorString;
};
}
