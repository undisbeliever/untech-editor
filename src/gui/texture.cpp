/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "texture.h"
#include "models/common/image.h"
#include "models/common/imagecache.h"
#include "models/common/iterators.h"

namespace UnTech::Gui {

static std::unique_ptr<Image> createMissingImageSymbol()
{
    const rgba red(255, 0, 0);
    constexpr unsigned imgSize = 32;

    auto img = std::make_unique<Image>(imgSize, imgSize);

    for (const auto i : range(1, imgSize)) {
        if (i < imgSize - 1) {
            img->setPixel(i, i, red);
            img->setPixel(i, 31 - i, red);
        }
        img->setPixel(i - 1, i, red);
        img->setPixel(i, i - 1, red);
        img->setPixel(i - 1, 31 - i, red);
        img->setPixel(i, 31 - i + 1, red);
    }

    return img;
}

void Texture::replaceWithMissingImageSymbol()
{
    static const std::unique_ptr<Image> symbol = createMissingImageSymbol();

    replace(*symbol);
}

void Texture::loadPngImage(const std::filesystem::path& filename)
{
    auto image = ImageCache::loadPngImage(filename);
    assert(image);

    if (!image->empty()) {
        this->replace(*image);
    }
    else {
        this->replaceWithMissingImageSymbol();
    }
}

}
