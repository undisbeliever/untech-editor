/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "image.h"
#include <memory>
#include <string>

namespace UnTech {

/**
 * The ImageCache provides an application wide cache for loading png images.
 *
 * The images held by this cache are stored for the lifetime of the program
 * (unless they are manually expired). This should not an issue as I do not
 * expect all the images used by this cache to exceed 50MB.
 *
 * The ImageCache WILL NEVER CHECK the file modification time to see if the
 * cached image is outdated.
 *
 * ImageCache is c++11 thread safe.
 */
class ImageCache {
private:
    ImageCache() = delete;

public:
    // Will never return a nullptr
    static const std::shared_ptr<const Image> loadPngImage(const std::string& filename);

    static void invalidateFilename(const std::string& filename);
    static void invalidateImageCache();
};
}
