/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/texture.h"
#include "models/common/image.h"

namespace UnTech::Gui {

constexpr static unsigned TILE_COLLISION_IMAGE_WIDTH = 16;
constexpr static unsigned TILE_COLLISION_IMAGE_HEIGHT = 32 * 16;

extern const Image tileCollisionImage;

const Texture& tileCollisionTypeTexture();

}
