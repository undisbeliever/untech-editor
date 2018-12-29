/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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

    RomOffsetPtr romPtr;
    bool dynamicTileset;

    // Mapping of tileId => Objects::charAttr bits
    std::vector<uint16_t> smallTilesCharAttr;
    std::vector<uint16_t> largeTilesCharAttr;
};

struct TilesetData {
    TilesetType tilesetType;

    FrameTilesetData staticTileset;
    std::vector<FrameTilesetData> dynamicTilesets;

    // Map of exportFrames index to dyanmicTiles index
    // if -1 then there are no dynamic tiles;
    std::vector<int> frameTilesets;

    const FrameTilesetData& tilesetForFrameId(unsigned frameId) const
    {
        int tilesetId = frameTilesets.at(frameId);
        return tilesetId < 0 ? staticTileset : dynamicTilesets.at(unsigned(tilesetId));
    }
};

TilesetData insertFrameSetTiles(const MetaSprite::FrameSet& frameSet, const TilesetLayout& tilesetLayout,
                                CompiledRomData& out);
}
}
}
