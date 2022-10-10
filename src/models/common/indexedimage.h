/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "aabb.h"
#include "exceptions.h"
#include "rgba.h"
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace UnTech {

/**
 * An image container class that contains a palette indexed image.
 *
 * The image contains a maximum of 256 colors.
 */
class IndexedImage {
public:
    using iterator = uint8_t*;
    using const_iterator = const uint8_t*;

private:
    const usize _size;
    const std::u8string _errorString;
    uint8_t* const _imageData;
    const size_t _dataSize;
    std::vector<rgba> _palette;

private:
    // Used to "privatize" the constructors while still allowing `std::make_shared`.
    struct PrivateToken {
    };

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

    static std::shared_ptr<IndexedImage> invalidImageWithErrorMessage(std::u8string&& error);

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
    IndexedImage(std::u8string&& errorString, PrivateToken);

    ~IndexedImage();

    /**
     * Returns true if the image is empty.
     */
    bool empty() const { return _imageData == nullptr; }

    inline usize size() const { return _size; }
    inline const std::u8string& errorString() const { return _errorString; }

    inline auto& palette() { return _palette; }
    inline const auto& palette() const { return _palette; }

    // Only sets pixels if `color < palette().size()`
    void fill(const uint8_t color);

    inline std::span<uint8_t> data() { return std::span{ _imageData, _dataSize }; }
    inline std::span<const uint8_t> data() const { return std::span{ _imageData, _dataSize }; }

    inline unsigned pixelsPerScanline() const { return _size.width; }

    std::span<uint8_t> scanline(unsigned y)
    {
        if (y >= _size.height) {
            throw out_of_range(u8"Image::scanline out of range");
        }
        assert(_imageData);
        return data().subspan(y * _size.width, _size.width);
    }

    std::span<const uint8_t> scanline(unsigned y) const
    {
        if (y >= _size.height) {
            throw out_of_range(u8"Image::scanline out of range");
        }
        assert(_imageData);
        return data().subspan(y * _size.width, _size.width);
    }

    inline uint8_t getPixel(unsigned x, unsigned y) const
    {
        if (x >= _size.width || y >= _size.height) {
            throw out_of_range(u8"Image::getPixel out of range");
        }
        assert(_imageData);
        return data()[x + y * _size.width];
    }

    inline void setPixel(unsigned x, unsigned y, const uint8_t p)
    {
        if (x >= _size.width || y >= _size.height) {
            throw out_of_range(u8"Image::setPixel out of range");
        }
        data()[x + y * _size.width] = p;
    }
};
}
