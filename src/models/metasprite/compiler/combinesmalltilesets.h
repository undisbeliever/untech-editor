/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "tilesetcompiler.h"
#include <array>
#include <unordered_map>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {
typedef std::unordered_map<unsigned, std::array<unsigned, 4>> SmallTileMap_t;

SmallTileMap_t combineSmallTilesets(const TileGraph_t& smallTileGraph);
}
}
}
