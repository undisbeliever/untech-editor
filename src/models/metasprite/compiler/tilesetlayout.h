/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetexportlist.h"
#include <array>
#include <cstdint>
#include <vector>

#pragma once

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

constexpr uint16_t INVALID_SMALL_TILE = 0xffff;
constexpr std::array<uint16_t, 4> INVALID_SMALL_TILES_ARRAY = { 0xffff, 0xffff, 0xffff, 0xffff };

// internal representation of a tile
// ids are the tile ids used by the FrameObjects
// if largeTileId == 0xffff then tile is small.
struct Tile16 {
    uint16_t largeTileId = INVALID_SMALL_TILE;
    std::array<uint16_t, 4> smallTileIds = INVALID_SMALL_TILES_ARRAY;

    bool isLarge() const { return largeTileId != INVALID_SMALL_TILE; }
    bool isSmall() const { return largeTileId == INVALID_SMALL_TILE; }

    bool operator==(const Tile16& o) const
    {
        return largeTileId == o.largeTileId && smallTileIds == o.smallTileIds;
    }
    bool operator!=(const Tile16& o) const
    {
        return !(*this == o);
    }
    bool operator<(const Tile16& o) const
    {
        return std::tie(largeTileId, smallTileIds)
               < std::tie(o.largeTileId, o.smallTileIds);
    }
};

struct TilesetLayout {
    std::vector<Tile16> staticTiles;
    std::vector<std::vector<Tile16>> dynamicTiles;
    TilesetType tilesetType;

    // Map of exportFrames index to dyanmicTiles index
    // if -1 then there are no dynamic tiles;
    std::vector<int> frameTilesets;
};

TilesetLayout layoutTiles(const MetaSprite::FrameSet& frameSet,
                          const std::vector<FrameListEntry>& exportFrames,
                          ErrorList& errorList);

}
}
}
