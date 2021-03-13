/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "animated-tileset.h"
#include "models/common/grid.h"

namespace UnTech::Resources {

void drawAnimatedTileset(grid<uint8_t>& image, const Resources::AnimatedTilesetData& animatedTileset, unsigned frameIndex);

}
