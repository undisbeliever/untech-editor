#pragma once

#include "compiler.h"
#include <list>
#include <set>

namespace UnTech {
namespace MetaSpriteCompiler {
typedef std::unordered_map<unsigned, std::array<unsigned, 4>> SmallTileMap_t;

SmallTileMap_t combineSmallTilesets(const TileGraph_t& smallTileGraph);
}
}
