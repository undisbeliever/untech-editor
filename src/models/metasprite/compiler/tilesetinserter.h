/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "compiler.h"
#include "../metasprite.h"
#include <cstdint>
#include <vector>

#pragma once

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

struct TilesetLayout;

struct FrameTilesetData {
    static constexpr uint16_t NULL_CHAR_ATTR = 0xffff;
    static constexpr unsigned MAX_TILES_PER_TILESET = 16;

    // index into `TilesetData::tiles` for each tile in this tileset
    std::vector<unsigned> tiles;

    bool dynamicTileset;

    // Mapping of tileId => Objects::charAttr bits
    std::vector<uint16_t> smallTilesCharAttr;
    std::vector<uint16_t> largeTilesCharAttr;
};

struct TilesetData {
    TilesetType tilesetType;

    std::vector<Snes::Tile16px> tiles;

    FrameTilesetData staticTileset;
    std::vector<FrameTilesetData> dynamicTilesets;

    // Map of exportFrames index to dyanmicTiles index
    // if -1 then there are no dynamic tiles;
    // ::TODO change to std::optional<uint16_t>::
    std::vector<int> frameTilesets;

    std::optional<unsigned> tilesetIndexForFrameId(unsigned frameId) const
    {
        int tilesetId = frameTilesets.at(frameId);
        return tilesetId >= 0 ? std::optional<int>(tilesetId) : std::nullopt;
    }

    const FrameTilesetData& getTileset(std::optional<unsigned> index) const
    {
        return index ? dynamicTilesets.at(*index) : staticTileset;
    }
};

TilesetData processTileset(const MetaSprite::FrameSet& frameSet, const TilesetLayout& tilesetLayout);

}
}
}
