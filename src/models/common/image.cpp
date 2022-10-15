/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "image.h"
#include "file.h"
#include "stringbuilder.h"
#include "models/common/u8strings.h"
#include "vendor/lodepng/lodepng.h"

#ifndef LODEPNG_COMPILE_ALLOCATORS
#error "Cannot use custom lodepng allocators"
#endif

namespace UnTech {

static_assert(sizeof(rgba) == 4, u8"rgba is the wrong size");

static constexpr size_t MAX_IMAGE_PIXELS = 1024 * 1024;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
static std::unique_ptr<rgba[]> allocateImageData(usize size)
{
    if (size.width <= 0 || size.height <= 0) {
        throw std::invalid_argument("Image size");
    }

    const size_t nPixels = size.width * size.height;
    if (nPixels > MAX_IMAGE_PIXELS) {
        throw std::invalid_argument("Image size too large");
    }

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
    return std::make_unique<rgba[]>(nPixels);
}

Image::Image(const usize size)
    : _imageData{ allocateImageData(size) }
    , _dataSize{ size.width * size.height }
    , _size{ size }
    , _errorString{}
{
    assert(_imageData);
    assert(_size.width > 0 && _size.height > 0);

    assert(uintptr_t(_imageData.get()) % alignof(rgba) == 0);
    assert(uintptr_t(_imageData.get()) % sizeof(rgba) == 0);
}

Image::Image(std::u8string&& errorString, Image::PrivateToken)
    : _imageData{ nullptr }
    , _dataSize{ 0 }
    , _size{ 0, 0 }
    , _errorString{ std::move(errorString) }
{
}

void Image::fill(const rgba& color)
{
    if (empty()) {
        return;
    }

    auto imageData = this->data();
    std::fill(imageData.begin(), imageData.end(), color);
}

std::shared_ptr<Image> Image::loadPngImage_shared(const std::filesystem::path& filename)
{
    static constexpr size_t IMAGE_FILE_LIMIT = 2 * 1024 * 1024;

    std::vector<uint8_t> fileData;
    try {
        fileData = File::readBinaryFile(filename, IMAGE_FILE_LIMIT);
    }
    catch (const std::exception& ex) {
        return invalidImageWithErrorMessage(convert_old_string(ex.what()));
    }

    std::shared_ptr<Image> image = nullptr;

    // Holds ownership of a C malloc buffer created by `lodepng_decode()`
    uint8_t* buffer = nullptr;

    usize size{};

    lodepng::State state{};
    state.decoder.zlibsettings.max_output_size = MAX_IMAGE_PIXELS * sizeof(rgba);

    state.info_raw.colortype = LCT_RGBA;
    state.info_raw.bitdepth = 8;

    const auto error = lodepng_decode(&buffer, &size.width, &size.height, &state,
                                      fileData.data(), fileData.size());

    if (!error && buffer) {
        image = std::make_shared<Image>(size);

        const size_t bufferSize = lodepng_get_raw_size(size.width, size.height, &state.info_raw);
        assert(bufferSize == size.width * size.height * sizeof(rgba));

        auto imageData = image->data();

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const std::span bufferSpan(reinterpret_cast<rgba*>(buffer), bufferSize / sizeof(rgba));

        // Copy `buffer` to `ret->imageData` and also transform all pixels with an alpha of 0 to rgba (0, 0, 0, 0)
        assert(imageData.size() == bufferSpan.size());
        std::transform(bufferSpan.begin(), bufferSpan.end(),
                       imageData.begin(),
                       [](rgba& c) { return (c.alpha != 0) ? c : rgba(0, 0, 0, 0); });
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

std::shared_ptr<Image> Image::invalidImageWithErrorMessage(std::u8string&& error)
{
    return std::make_shared<Image>(std::move(error), PrivateToken{});
}

}
