/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "framesetexportlist.h"
#include "romtiledata.h"
#include "../errorlist.h"
#include "../metasprite.h"
#include "models/common/vectorset.h"
#include <array>
#include <cstdint>
#include <list>
#include <unordered_map>
#include <vector>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

typedef std::unordered_map<unsigned, std::vector<const MetaSprite::Frame*>> TileGraph_t;
typedef std::unordered_map<unsigned, std::array<uint16_t, 4>> SmallTileMap_t;

struct FrameListEntry;
struct AnimationListEntry;

struct FrameTileset {
    RomOffsetPtr tilesetOffset;

    // the uint16_t matches the charattr bits of the frameobject data.
    std::unordered_map<unsigned, uint16_t> smallTilesetMap;
    std::unordered_map<unsigned, uint16_t> largeTilesetMap;
};

struct FrameSetTilesets {
    RomOffsetPtr tilesetOffset;
    TilesetType tilesetType;

    std::unordered_map<const MetaSprite::Frame*, FrameTileset&> frameMap;

    // Holds the tileset data used by the FrameMap
    std::list<FrameTileset> tilesets;
};

class TilesetCompiler {
public:
    const static unsigned DEFAULT_TILE_BLOCK_SIZE = 8 * 1024;

    struct Tile16;
    struct FrameTilesetData;
    struct DynamicTilesetData;

public:
    TilesetCompiler(ErrorList& errorList, unsigned tilesetBlockSize);
    TilesetCompiler(const TilesetCompiler&) = delete;

    void writeToIncFile(std::ostream& out) const;

    FrameSetTilesets generateTilesets(const FrameSetExportList& exportList);

private:
    void validateExportList(const FrameSetExportList& exportList);

    SmallTileMap_t buildSmallTileMap(const std::vector<FrameListEntry>& frameEntries);

    vectorset<Tile16> fixedTilesetData(const std::vector<FrameListEntry>& frameEntries,
                                       const SmallTileMap_t& smallTileMap) const;

    DynamicTilesetData dynamicTilesetData(const std::vector<FrameListEntry>& frameEntries,
                                          const SmallTileMap_t& smallTileMap,
                                          const TilesetType tilesetType) const;

    void addFrameToTileset(vectorset<Tile16>& tiles,
                           const MetaSprite::Frame& frame,
                           const SmallTileMap_t& smallTileMap) const;

    vectorset<Tile16> calculateStaticTiles(const std::vector<FrameTilesetData>& ftVector,
                                           const TilesetType tilesetType) const;

    std::vector<std::pair<Tile16, unsigned>> countTileUsage(
        const std::vector<FrameTilesetData>& ftVector) const;

    FrameSetTilesets buildFixedTileset(const MetaSprite::FrameSet& frameSet,
                                       const TilesetType tilesetType,
                                       const vectorset<Tile16>& tiles);

    FrameSetTilesets buildDynamicTileset(const MetaSprite::FrameSet& frameSet,
                                         const TilesetType tilesetType,
                                         const DynamicTilesetData& tileset);

    FrameTileset buildTileset(const MetaSprite::FrameSet& frameSet,
                              const TilesetType tilesetType,
                              const vectorset<Tile16>& tiles,
                              const unsigned tileOffset = 0);

private:
    ErrorList& _errorList;

    RomTileData _tileData;
    RomIncData _tilesetData;
};
}
}
}
