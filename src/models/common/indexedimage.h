/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "aabb.h"
#include "rgba.h"
#include <cstdint>
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
    ~IndexedImage() = default;
    IndexedImage(const IndexedImage&) = default;
    IndexedImage(IndexedImage&&) = default;
    IndexedImage& operator=(const IndexedImage&) = default;
    IndexedImage& operator=(IndexedImage&&) = default;

    IndexedImage();
    IndexedImage(const usize& size);
    IndexedImage(unsigned width, unsigned height);

    inline usize size() const { return _size; }
    inline std::string errorString() const { return _errorString; }

    inline auto& palette() { return _palette; }
    inline const auto& palette() const { return _palette; }

    void erase();

    void fill(uint8_t color);

    /**
     * Loads a PNG image from a filename.
     *
     * NOTE: The image cannot be loaded if it contains more than 256 colors.
     *
     * This overrides the current image.
     *
     * If the image cannot be loaded then:
     *   - return false
     *   - the image is erased.
     *   _ errorString is set.
     */
    bool loadPngImage(const std::string& filename);

    /**
     * Returns true if the image is empty.
     */
    bool empty() const { return _size.width == 0 || _size.height == 0; }

    inline uint8_t* data() { return _imageData.data(); }
    inline const uint8_t* data() const { return _imageData.data(); }

    inline uint8_t* scanline(unsigned y) { return data() + (y * _size.width); }
    inline const uint8_t* scanline(unsigned y) const { return data() + (y * _size.width); }

    inline uint8_t getPixel(unsigned x, unsigned y) const
    {
        return *(data() + x + (y * _size.width));
    }

private:
    usize _size;
    std::vector<uint8_t> _imageData;
    std::vector<rgba> _palette;
    std::string _errorString;
};
}
