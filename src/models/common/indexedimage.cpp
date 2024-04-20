/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "indexedimage.h"
#include "file.h"
#include "stringbuilder.h"
#include "models/common/u8strings.h"
#include "vendor/lodepng/lodepng.h"

#ifndef LODEPNG_COMPILE_ALLOCATORS
#error "Cannot use custom lodepng allocators"
#endif

namespace UnTech {

static_assert(sizeof(rgba) == 4, "rgba is the wrong size");

static constexpr size_t MAX_IMAGE_PIXELS = 1024 * 1024;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
static std::unique_ptr<uint8_t[]> allocateIndexedImageData(usize size)
{
    if (size.width <= 0 || size.height <= 0) {
        throw std::invalid_argument("IndexedImage size");
    }

    const size_t nPixels = size.width * size.height;
    if (nPixels > MAX_IMAGE_PIXELS) {
        throw std::invalid_argument("IndexedImage size too large");
    }

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
    return std::make_unique<uint8_t[]>(nPixels);
}

IndexedImage::IndexedImage(const usize size)
    : _imageData{ allocateIndexedImageData(size) }
    , _dataSize{ size.width * size.height }
    , _size{ size }
    , _palette{}
    , _errorString{}
{
    assert(_imageData);
    assert(_size.width > 0 && _size.height > 0);
}

IndexedImage::IndexedImage(std::u8string&& errorString, IndexedImage::PrivateToken)
    : _imageData{ nullptr }
    , _dataSize{ 0 }
    , _size{ 0, 0 }
    , _palette{}
    , _errorString{ std::move(errorString) }
{
}

void IndexedImage::fill(uint8_t color)
{
    if (color < _palette.size()) {
        auto imageData = this->data();
        std::fill(imageData.begin(), imageData.end(), color);
    }
}

std::shared_ptr<IndexedImage> IndexedImage::loadPngImage_shared(const std::filesystem::path& filename)
{
    static constexpr size_t IMAGE_FILE_LIMIT = 2 * 1024 * 1024;

    std::vector<uint8_t> fileData;
    try {
        fileData = File::readBinaryFile(filename, IMAGE_FILE_LIMIT);
    }
    catch (const std::exception& ex) {
        return invalidImageWithErrorMessage(convert_old_string(ex.what()));
    }

    std::shared_ptr<IndexedImage> image = nullptr;

    // Holds ownership of a C malloc buffer created by `lodepng_decode()`
    uint8_t* buffer = nullptr;

    usize size{};

    lodepng::State state{};
    state.decoder.zlibsettings.max_output_size = MAX_IMAGE_PIXELS;

    state.decoder.color_convert = true;
    state.info_raw.colortype = LodePNGColorType::LCT_PALETTE;
    state.info_raw.bitdepth = 8;

    const auto error = lodepng_decode(&buffer, &size.width, &size.height,
                                      &state,
                                      fileData.data(), fileData.size());

    if (!error && state.info_png.color.colortype == LodePNGColorType::LCT_PALETTE) {
        image = std::make_shared<IndexedImage>(size);

        const size_t bufferSize = lodepng_get_raw_size(size.width, size.height, &state.info_raw);
        assert(bufferSize == size.width * size.height);

        assert(buffer);
        const std::span bufferSpan(buffer, bufferSize / sizeof(uint8_t));
        std::copy(bufferSpan.begin(), bufferSpan.end(), image->data().begin());

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const std::span pngPal(reinterpret_cast<rgba*>(state.info_png.color.palette),
                               state.info_png.color.palettesize);

        image->_palette.insert(image->_palette.begin(), pngPal.begin(), pngPal.end());
    }
    else if (!error) {
        image = invalidImageWithErrorMessage(stringBuilder(filename.u8string(), u8": Not a indexed png file"));
    }
    else {
        image = invalidImageWithErrorMessage(stringBuilder(filename.u8string(), u8": ",
                                                           convert_old_string(lodepng_error_text(error))));
    }

    if (buffer) {
        // NOLINTNEXTLINE(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)
        free(buffer);
        buffer = nullptr;
    }

    assert(image);
    return image;
}

std::shared_ptr<IndexedImage> IndexedImage::invalidImageWithErrorMessage(std::u8string&& error)
{
    return std::make_shared<IndexedImage>(std::move(error), PrivateToken{});
}

}
