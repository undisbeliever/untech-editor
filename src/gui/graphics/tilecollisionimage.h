/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/texture.h"
#include "models/common/image.h"

namespace UnTech::Gui {

constexpr static unsigned TILE_COLLISION_IMAGE_WIDTH = 16;
constexpr static unsigned TILE_COLLISION_IMAGE_HEIGHT = 32 * 16;

// ::TODO see if I can use a 1bpp or 8bpp image for tileCollisionImage::
extern const Image tileCollisionImage;

Texture& tileCollisionTypeTexture();

}
