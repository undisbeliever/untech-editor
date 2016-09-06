#pragma once

#include "errorlist.h"
#include "framesetexportlist.h"
#include "romtiledata.h"
#include "models/metasprite.h"
#include <array>
#include <cstdint>
#include <list>
#include <set>
#include <unordered_map>
#include <vector>

namespace UnTech {
namespace MetaSpriteCompiler {

typedef std::unordered_map<unsigned, std::vector<const MetaSprite::Frame*>> TileGraph_t;

struct FrameListEntry;
struct AnimationListEntry;

struct FrameTileset {
    RomOffsetPtr tilesetOffset;

    // the uint16_t matches the charattr bits of the frameobject data.
    std::unordered_map<unsigned, uint16_t> smallTilesetMap;
    std::unordered_map<unsigned, uint16_t> largeTilesetMap;
};

struct FrameTilesetList {
    MetaSpriteCommon::TilesetType tilesetType;

    std::unordered_map<const MetaSprite::Frame*, FrameTileset&> frameMap;

    // Holds the tileset data used by the FrameMap
    std::list<FrameTileset> tilesets;
};

class TilesetCompiler {
public:
    const static unsigned DEFAULT_TILE_BLOCK_SIZE = 8 * 1024;

public:
    TilesetCompiler(ErrorList& errorList, unsigned tilesetBlockSize);
    TilesetCompiler(const TilesetCompiler&) = delete;

    void writeToIncFile(std::ostream& out) const;

    FrameTilesetList generateTilesetList(const FrameSetExportList& exportList);

private:
    FrameTilesetList generateDynamicTilesets(const MetaSprite::FrameSet& frameSet,
                                             const MetaSpriteCommon::TilesetType& tilesetType,
                                             const TileGraph_t& largeTileGraph,
                                             const TileGraph_t& smallTileGraph);

    FrameTilesetList generateFixedTileset(const MetaSprite::FrameSet& frameSet,
                                          const MetaSpriteCommon::TilesetType& tilesetType,
                                          const TileGraph_t& largeTileGraph,
                                          const TileGraph_t& smallTileGraph);

    void buildTileset(FrameTileset& tileset,
                      const MetaSprite::FrameSet& frameSet,
                      const MetaSpriteCommon::TilesetType& tilesetType,
                      const std::set<unsigned>& largeTiles,
                      const std::set<std::array<unsigned, 4>>& smallTiles);

    void buildTileset(FrameTileset& tileset,
                      const MetaSprite::FrameSet& frameSet,
                      const MetaSpriteCommon::TilesetType& tilesetType,
                      const std::set<unsigned>& largeTiles,
                      const std::set<unsigned>& smallTiles);

private:
    ErrorList& _errorList;

    RomTileData _tileData;
    RomIncData _tilesetData;
};
}
}
