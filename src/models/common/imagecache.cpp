/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "imagecache.h"
#include <mutex>
#include <unordered_map>

namespace UnTech {
class ImageCachePrivate {
    friend class UnTech::ImageCache;
    using ImageCacheMap_t = std::unordered_map<std::filesystem::path::string_type, const std::shared_ptr<const Image>>;

    static ImageCachePrivate& instance()
    {
        static ImageCachePrivate i;
        return i;
    }

    const std::shared_ptr<const Image> loadPngImage(const std::filesystem::path& filename)
    {
        std::lock_guard<std::mutex> guard(mutex);

        const auto abs = std::filesystem::absolute(filename);
        const auto& fn = abs.native();

        auto it = cache.find(fn);
        if (it != cache.end()) {
            return it->second;
        }
        else {
            std::shared_ptr<Image> image = std::make_shared<Image>();
            image->loadPngImage(abs);
            cache.insert({ fn, image });

            return image;
        }
    }

    void invalidateFilename(const std::filesystem::path& filename)
    {
        std::lock_guard<std::mutex> guard(mutex);

        const auto abs = std::filesystem::absolute(filename);
        const auto& fn = abs.native();

        auto it = cache.find(fn);
        if (it != cache.end()) {
            cache.erase(it);
        }
    }

    void invalidateImageCache()
    {
        std::lock_guard<std::mutex> guard(mutex);

        cache.clear();
    }

private:
    ImageCachePrivate() = default;

    std::mutex mutex;
    ImageCacheMap_t cache;
};
}

using namespace UnTech;

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
