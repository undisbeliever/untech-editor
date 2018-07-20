/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "combinesmalltiles.h"
#include "framesetexportlist.h"
#include "romtiledata.h"
#include "../errorlist.h"
#include "../metasprite.h"
#include "models/common/vectorset.h"
#include <array>
#include <cstdint>
#include <list>
#include <vector>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

struct FrameListEntry;
struct AnimationListEntry;

struct FrameTileset {
    static const uint16_t NULL_CHAR_ATTR;

    FrameTileset(const MetaSprite::FrameSet& fs);

    void merge(const FrameTileset& ft);
    uint16_t charAttr(ObjectSize objSize, unsigned tileId) const;

    RomOffsetPtr tilesetOffset;

    // Mapping of tileId => Objects::charAttr bits
    std::vector<uint16_t> smallTilesCharAttr;
    std::vector<uint16_t> largeTilesCharAttr;
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
                                         const DynamicTilesetData& tilesetData);

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
