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
#include <span>
#include <stdexcept>
#include <string>

namespace UnTech {

/**
 * A simple image container class that contains a 32bpp RGBA image.
 */
class Image {
public:
    using iterator = rgba*;
    using const_iterator = const rgba*;

private:
    const usize _size;
    const std::string _errorString;
    rgba* const _imageData;
    const size_t _dataSize;

public:
    /**
     * Loads a PNG image from a filename.
     *
     * Will never return null.
     *
     * NOTE: This method may refuse to load the image if it is too large.
     *
     * NOTE: This method will transform all pixels with an alpha value to 0 to
     *       `rgba(0, 0, 0, 0)`.
     *
     * If the image cannot be loaded an empty image with an `errorString` set is returned.
     */
    static std::shared_ptr<Image> loadPngImage_shared(const std::filesystem::path& filename);

    static std::shared_ptr<Image> invalidImageWithErrorMessage(std::string&& error);

private:
    // Used to "privatize" the constructors while still allowing `std::make_shared`.
    struct PrivateToken {
    };

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

    std::span<rgba> data() { return std::span{ _imageData, _dataSize }; }
    std::span<const rgba> data() const { return std::span{ _imageData, _dataSize }; }

    inline unsigned pixelsPerScanline() const { return _size.width; }

    std::span<rgba> scanline(unsigned y)
    {
        if (y >= _size.height) {
            throw std::out_of_range("Image::scanline out of range");
        }
        assert(_imageData);
        return std::span(_imageData + (y * _size.width), _size.width);
    }

    std::span<const rgba> scanline(unsigned y) const
    {
        if (y >= _size.height) {
            throw std::out_of_range("Image::scanline out of range");
        }
        assert(_imageData);
        return std::span(_imageData + (y * _size.width), _size.width);
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
};
}
