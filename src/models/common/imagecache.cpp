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
    using ImageCacheMap_t = std::unordered_map<std::string, const std::shared_ptr<const Image>>;

    static ImageCachePrivate& instance()
    {
        static ImageCachePrivate i;
        return i;
    }

    const std::shared_ptr<const Image> loadPngImage(const std::string& filename)
    {
        std::lock_guard<std::mutex> guard(mutex);

        auto it = cache.find(filename);
        if (it != cache.end()) {
            return it->second;
        }
        else {
            std::shared_ptr<Image> image = std::make_unique<Image>();
            image->loadPngImage(filename);
            cache.insert({ filename, image });

            return image;
        }
    }

    void invalidateFilename(const std::string& filename)
    {
        std::lock_guard<std::mutex> guard(mutex);

        auto it = cache.find(filename);
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

const std::shared_ptr<const Image> ImageCache::loadPngImage(const std::string& filename)
{
    return ImageCachePrivate::instance().loadPngImage(filename);
}

void ImageCache::invalidateFilename(const std::string& filename)
{
    ImageCachePrivate::instance().invalidateFilename(filename);
}

void ImageCache::invalidateImageCache()
{
    ImageCachePrivate::instance().invalidateImageCache();
}
