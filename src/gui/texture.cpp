/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "texture.h"
#include "models/common/image.h"

namespace UnTech::Gui {

const UnTech::Image Texture::missingImageSymbol = []() {
    const rgba red(255, 0, 0);

    UnTech::Image img(32, 32);
    for (unsigned i = 1; i < 32; i++) {
        if (i < 31) {
            img.setPixel(i, i, red);
            img.setPixel(i, 31 - i, red);
        }
        img.setPixel(i - 1, i, red);
        img.setPixel(i, i - 1, red);
        img.setPixel(i - 1, 31 - i, red);
        img.setPixel(i, 31 - i + 1, red);
    }
    return img;
}();

}
