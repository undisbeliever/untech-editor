/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "framesetexportlist.h"
#include "tilesetinserter.h"

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

// Mapping of small tileId => The four small tiles that combine to form a Tile16.
typedef std::vector<std::array<uint16_t, 4>> SmallTileMap_t;

SmallTileMap_t buildSmallTileMap(const MetaSprite::FrameSet& frameSet,
                                 const std::vector<FrameListEntry>& frameEntries);
}
}
}
