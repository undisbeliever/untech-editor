/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "aabb.h"
#include "rgba.h"
#include <cassert>
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
private:
    // Used to "privatize" the constructors while still allowing `std::make_shared`.
    struct PrivateToken {
    };

public:
    using iterator = rgba*;
    using const_iterator = const rgba*;

public:
    /**
     * Loads a PNG image from a filename.
     *
     * Will never return null.
     *
     * NOTE: This method will transform all pixels with an alpha value to 0 to
     *       `rgba(0, 0, 0, 0)`.
     *
     * If the image cannot be loaded an empty image with an `errorString` set is returned.
     */
    static std::shared_ptr<Image> loadPngImage_shared(const std::filesystem::path& filename);

    static std::shared_ptr<Image> invalidImageWithErrorMessage(std::string&& error);

private:
    const usize _size;
    const std::string _errorString;
    rgba* const _imageData;
    rgba* const _imageDataEnd;

public:
    // Images cannot be moved or copied.
    Image(const Image&) = delete;
    Image(Image&&) = delete;
    Image& operator=(const Image&) = delete;
    Image& operator=(Image&&) = delete;

    Image(const usize size);
    Image(unsigned width, unsigned height)
        : Image(usize(width, height))
    {
    }

    // "Private" constructors for use with `std::make_shared`.
    // Takes ownership of `data`
    Image(const usize size, rgba*&& data, PrivateToken);
    Image(std::string&& errorString, PrivateToken);

    ~Image();

    /**
     * Returns true if the image is empty.
     */
    bool empty() const { return _imageData == nullptr; }

    usize size() const { return _size; }
    std::string errorString() const { return _errorString; }

    void fill(const rgba& color);

    inline size_t dataSize() const { return _size.width * _size.height; }

    inline rgba* data() { return _imageData; }
    inline const rgba* data() const { return _imageData; }

    inline rgba* dataEnd() { return _imageDataEnd; }
    inline const rgba* dataEnd() const { return _imageDataEnd; }

    inline unsigned pixelsPerScanline() const { return _size.width; }

    inline rgba* scanline(unsigned y)
    {
        if (y >= _size.height) {
            throw std::out_of_range("Image::scanline out of range");
        }
        assert(_imageData);
        return _imageData + (y * pixelsPerScanline());
    }

    inline const rgba* scanline(unsigned y) const
    {
        if (y >= _size.height) {
            throw std::out_of_range("Image::scanline out of range");
        }
        assert(_imageData);
        return _imageData + (y * pixelsPerScanline());
    }

    inline rgba getPixel(unsigned x, unsigned y) const
    {
        if (x >= _size.width || y >= _size.height) {
            throw std::out_of_range("Image::getPixel out of range");
        }
        assert(_imageData);
        return _imageData[x + y * pixelsPerScanline()];
    }

    inline void setPixel(unsigned x, unsigned y, const rgba& p)
    {
        if (x >= _size.width || y >= _size.height) {
            throw std::out_of_range("Image::setPixel out of range");
        }
        assert(_imageData);
        _imageData[x + y * pixelsPerScanline()] = p;
    }

    iterator begin() { return _imageData; }
    iterator end() { return _imageDataEnd; }

    const_iterator begin() const { return _imageData; }
    const_iterator end() const { return _imageDataEnd; }

    const_iterator cbegin() const { return _imageData; }
    const_iterator cend() const { return _imageDataEnd; }
};
}
