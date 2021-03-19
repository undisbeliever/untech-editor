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
#include <string>
#include <vector>

namespace UnTech {

/**
 * An image container class that contains a palette indexed image.
 *
 * The image contains a maximum of 256 colors.
 */
class IndexedImage {
private:
    // Used to "privatize" the constructors while still allowing `std::make_shared`.
    struct PrivateToken {
    };

public:
    using iterator = uint8_t*;
    using const_iterator = const uint8_t*;

private:
    const usize _size;
    const std::string _errorString;
    uint8_t* const _imageData;
    uint8_t* const _imageDataEnd;
    std::vector<rgba> _palette;

public:
    /**
     * Loads a PNG image from a filename.
     *
     * Will never return null.
     *
     * NOTE: The image cannot be loaded if it contains more than 256 colors.
     *
     * NOTE: This method may refuse to load the image if it is too large.
     *
     * If the image cannot be loaded an empty image with an `errorString` set is returned.
     */
    static std::shared_ptr<IndexedImage> loadPngImage_shared(const std::filesystem::path& filename);

    static std::shared_ptr<IndexedImage> invalidImageWithErrorMessage(std::string&& error);

public:
    // IndexedImage cannot be moved or copied.
    IndexedImage(const IndexedImage&) = delete;
    IndexedImage(IndexedImage&&) = delete;
    IndexedImage& operator=(const IndexedImage&) = delete;
    IndexedImage& operator=(IndexedImage&&) = delete;

    IndexedImage(const usize size);
    IndexedImage(unsigned width, unsigned height)
        : IndexedImage(usize(width, height))
    {
    }

    // "Private" constructors for use with `std::make_shared`.
    // Takes ownership of `data`
    IndexedImage(const usize size, uint8_t*&& data, PrivateToken);
    IndexedImage(std::string&& errorString, PrivateToken);

    ~IndexedImage();

    /**
     * Returns true if the image is empty.
     */
    bool empty() const { return _imageData == nullptr; }

    inline usize size() const { return _size; }
    inline const std::string& errorString() const { return _errorString; }

    inline auto& palette() { return _palette; }
    inline const auto& palette() const { return _palette; }

    // Only sets pixels if `color < palette().size()`
    void fill(const uint8_t color);

    inline uint8_t* data() { return _imageData; }
    inline const uint8_t* data() const { return _imageData; }

    inline uint8_t* dataEnd() { return _imageDataEnd; }
    inline const uint8_t* dataEnd() const { return _imageDataEnd; }

    inline unsigned pixelsPerScanline() const { return _size.width; }

    inline uint8_t* scanline(unsigned y)
    {
        if (y >= _size.height) {
            throw std::out_of_range("Image::scanline out of range");
        }
        assert(_imageData);
        return _imageData + (y * pixelsPerScanline());
    }

    inline const uint8_t* scanline(unsigned y) const
    {
        if (y >= _size.height) {
            throw std::out_of_range("Image::scanline out of range");
        }
        assert(_imageData);
        return _imageData + (y * pixelsPerScanline());
    }

    inline uint8_t getPixel(unsigned x, unsigned y) const
    {
        if (x >= _size.width || y >= _size.height) {
            throw std::out_of_range("Image::getPixel out of range");
        }
        assert(_imageData);
        return _imageData[x + y * pixelsPerScanline()];
    }

    inline void setPixel(unsigned x, unsigned y, const uint8_t p)
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
