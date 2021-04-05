/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "indexedimage.h"
#include "file.h"
#include "stringbuilder.h"
#include "vendor/lodepng/lodepng.h"

#ifndef LODEPNG_COMPILE_ALLOCATORS
#error "Cannot use custom lodepng allocators"
#endif

namespace UnTech {

IndexedImage::IndexedImage(const usize size)
    : _size(size)
    , _errorString()
    , _imageData(reinterpret_cast<uint8_t*>(malloc(size.width * size.height)))
    , _imageDataEnd(_imageData + (size.width * size.height))
{
    assert(_size.width > 0 && _size.height > 0);

    if (_imageData == nullptr) {
        throw std::bad_alloc();
    }
}

IndexedImage::IndexedImage(const usize size, uint8_t*&& data, IndexedImage::PrivateToken)
    : _size(size)
    , _errorString()
    , _imageData(data)
    , _imageDataEnd(_imageData + (size.width * size.height))
{
    assert(_size.width > 0 && _size.height > 0);

    if (_imageData == nullptr) {
        throw std::bad_alloc();
    }
}

IndexedImage::IndexedImage(std::string&& errorString, IndexedImage::PrivateToken)
    : _size(0, 0)
    , _errorString(std::move(errorString))
    , _imageData(nullptr)
    , _imageDataEnd(nullptr)
    , _palette()
{
}

IndexedImage::~IndexedImage()
{
    if (_imageData) {
        // Cannot use delete[] here, `lodepng_decode` allocates memory using `malloc`.
        free(_imageData);
    }
}

void IndexedImage::fill(uint8_t color)
{
    if (color < _palette.size()) {
        std::fill(_imageData, _imageDataEnd, color);
    }
}

std::shared_ptr<IndexedImage> IndexedImage::loadPngImage_shared(const std::filesystem::path& filename)
{
    static constexpr size_t IMAGE_FILE_LIMIT = 2 * 1024 * 1024;
    static constexpr size_t MAX_IMAGE_PIXELS = 1024 * 1024;

    usize size;
    uint8_t* pixels = nullptr;

    try {
        const auto fileData = File::readBinaryFile(filename, IMAGE_FILE_LIMIT);

        lodepng::State state;
        state.decoder.zlibsettings.max_output_size = MAX_IMAGE_PIXELS;

        state.decoder.color_convert = true;
        state.info_raw.colortype = LodePNGColorType::LCT_PALETTE;
        state.info_raw.bitdepth = 8;

        const auto error = lodepng_decode(&pixels, &size.width, &size.height,
                                          &state,
                                          fileData.data(), fileData.size());

        if (!error) {
            const size_t buffersize = lodepng_get_raw_size(size.width, size.height, &state.info_raw);
            assert(buffersize == size.width * size.height);

            if (state.info_png.color.colortype == LodePNGColorType::LCT_PALETTE) {
                auto image = std::make_shared<IndexedImage>(size, std::move(pixels), PrivateToken{});

                const rgba* pngPal = reinterpret_cast<rgba*>(state.info_png.color.palette);
                const unsigned pngPalSize = state.info_png.color.palettesize;

                image->_palette.insert(image->_palette.begin(), pngPal, pngPal + pngPalSize);

                return image;
            }
            else {
                return invalidImageWithErrorMessage(stringBuilder(filename.string(), ": Not a indexed png file"));
            }
        }
        else {
            if (pixels) {
                free(pixels);
            }

            return invalidImageWithErrorMessage(stringBuilder(filename.string(), ": ", lodepng_error_text(error)));
        }
    }
    catch (const std::exception& ex) {
        return invalidImageWithErrorMessage(ex.what());
    }
}

std::shared_ptr<IndexedImage> IndexedImage::invalidImageWithErrorMessage(std::string&& error)
{
    return std::make_shared<IndexedImage>(std::move(error), PrivateToken{});
}

}
