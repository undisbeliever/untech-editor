/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "imagecache.h"
#include "mutex_wrapper.h"
#include <cassert>
#include <mutex>
#include <unordered_map>

using namespace std::string_literals;

namespace UnTech {

class ImageCachePrivate {
    friend class UnTech::ImageCache;
    using ImageCacheMap_t = std::unordered_map<std::filesystem::path::string_type, const std::shared_ptr<const Image>>;

private:
    mutex<ImageCacheMap_t> _cache;

    static const std::shared_ptr<const Image> BLANK_IMAGE;

private:
    ImageCachePrivate() = default;

public:
    static ImageCachePrivate& instance()
    {
        static ImageCachePrivate i;
        return i;
    }

    const std::shared_ptr<const Image> loadPngImage(const std::filesystem::path& filename)
    {
        if (filename.empty()) {
            return BLANK_IMAGE;
        }

        std::error_code ec;
        const auto abs = std::filesystem::absolute(filename, ec);
        if (ec) {
            return BLANK_IMAGE;
        }

        return _cache.access_and_return_const_shared_ptr<Image>([&](auto& cache) {
            const auto& fn = abs.native();

            auto it = cache.find(fn);
            if (it != cache.end()) {
                return it->second;
            }
            else {
                std::shared_ptr<const Image> image = Image::loadPngImage_shared(abs);
                assert(image);

                cache.insert({ fn, image });

                return image;
            }
        });
    }

    void invalidateFilename(const std::filesystem::path& filename)
    {
        std::error_code ec;
        const auto abs = std::filesystem::absolute(filename, ec);
        if (ec) {
            return;
        }

        const auto& fn = abs.native();

        _cache.access([&](auto& cache) {
            auto it = cache.find(fn);
            if (it != cache.end()) {
                cache.erase(it);
            }
        });
    }

    void invalidateImageCache()
    {
        _cache.access([&](auto& cache) {
            cache.clear();
        });
    }
};

const std::shared_ptr<const Image> ImageCachePrivate::BLANK_IMAGE = Image::invalidImageWithErrorMessage(u8"Invalid filename"s);

const std::shared_ptr<const Image> ImageCache::loadPngImage(const std::filesystem::path& filename)
{
    return ImageCachePrivate::instance().loadPngImage(filename);
}

void ImageCache::invalidateFilename(const std::filesystem::path& filename)
{
    ImageCachePrivate::instance().invalidateFilename(filename);
}

void ImageCache::invalidateImageCache()
{
    ImageCachePrivate::instance().invalidateImageCache();
}

}
