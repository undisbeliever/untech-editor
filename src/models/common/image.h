#pragma once

#include "aabb.h"
#include "rgba.h"
#include <cstdint>
#include <string>
#include <vector>

namespace UnTech {

/**
 * A simple image container class that contains a 32bpp RGBA image.
 */
class Image {
public:
    Image();
    Image(const usize& size);
    Image(unsigned width, unsigned height);

    Image(const Image&) = delete;

    ~Image() = default;

    usize size() const { return _size; }
    std::string errorString() const { return _errorString; }

    void erase();

    void fill(const rgba& color);

    /**
     * Loads a PNG image from a filename.
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

    inline rgba* data()
    {
        return reinterpret_cast<rgba*>(_imageData.data());
    }

    inline rgba* scanline(unsigned y)
    {
        return data() + (y * _size.width);
    }

    inline const rgba* data() const
    {
        return reinterpret_cast<const rgba*>(_imageData.data());
    }

    inline const rgba* scanline(unsigned y) const
    {
        return data() + (y * _size.width);
    }

    inline rgba getPixel(unsigned x, unsigned y)
    {
        return *(data() + x + (y * _size.width));
    }

private:
    usize _size;
    std::vector<unsigned char> _imageData;
    std::string _errorString;
};
}
