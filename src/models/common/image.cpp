#include "image.h"
#include "vendor/lodepng/lodepng.h"
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
}

Image::Image(unsigned width, unsigned height)
    : _size(width, height)
    , _imageData(width * height * sizeof(rgba))
    , _errorString()
{
}

void Image::erase()
{
    _size = usize(0, 0);
    _imageData.clear();
}

void Image::fill(const rgba& color)
{
    auto* ptr = data();
    auto* ptrEnd = scanline(_size.height);

    while (ptr < ptrEnd) {
        *ptr++ = color;
    }
}

bool Image::loadPngImage(const std::string& filename)
{
    auto error = lodepng::decode(_imageData, _size.width, _size.height, filename);

    if (error) {
        erase();

        std::stringstream msg;
        msg << filename << ": " << lodepng_error_text(error);
        _errorString = msg.str();

        return false;
    }

    return true;
}
