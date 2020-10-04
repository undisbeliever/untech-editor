/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "models/common/grid.h"
#include "models/common/idstring.h"
#include "models/resources/animated-tileset.h"
#include "models/resources/animation-frames-input.h"
#include <memory>
#include <string>
#include <vector>

namespace UnTech {
namespace MetaTiles {

struct InteractiveTilesData;

// ::KUDOS Christopher Hebert for the slope names::
// ::: Reconstructing Cave Story: Slope Theory::
// ::: https://www.youtube.com/watch?v=ny14i0GxGZw ::
//
// Order MUST MATCH untech engine.
enum class TileCollisionType : uint8_t {
    SOLID,
    DOWN_RIGHT_SLOPE, // right and down sides are the biggest, fall down to collide, walk right to ascend
    DOWN_LEFT_SLOPE,
    DOWN_RIGHT_SHORT_SLOPE,
    DOWN_RIGHT_TALL_SLOPE,
    DOWN_LEFT_TALL_SLOPE,
    DOWN_LEFT_SHORT_SLOPE,
    UP_PLATFORM,
    EMPTY,
    DOWN_PLATFORM,
    UP_RIGHT_SLOPE,
    UP_LEFT_SLOPE,
    UP_RIGHT_SHORT_SLOPE,
    UP_RIGHT_TALL_SLOPE,
    UP_LEFT_TALL_SLOPE,
    UP_LEFT_SHORT_SLOPE,

    END_SLOPE,
};
constexpr uint8_t N_TILE_COLLISONS = 17;

struct TilePriorities {
    constexpr static unsigned PRIORITIES_PER_TILE = 4;

    // Cannot use a std::array<bool, N> for tile priority it's not bit packed.
    // Cannot use a std::bitset<N> as it cannot be converted to a byte array/vector for serialization.
    // Bit order matches `MetaTileTilesetData::tileMap` order.
    std::array<uint8_t, N_METATILES * PRIORITIES_PER_TILE / 8> data;

    TilePriorities()
        : data{}
    {
    }

    static unsigned bitIndex(unsigned metaTile, unsigned subTile)
    {
        metaTile %= N_METATILES;

        const bool sx = subTile & 1;
        const bool sy = (subTile >> 1) & 1;

        const unsigned x = (metaTile % TILESET_WIDTH) * 2 + sx;
        const unsigned y = (metaTile & ~(TILESET_WIDTH - 1)) * 4 + sy * TILESET_WIDTH * 2;

        return y + x;
    }

    bool getTilePriority(unsigned metaTile, unsigned subTile) const
    {
        const unsigned bi = bitIndex(metaTile, subTile);
        const unsigned byteIndex = bi / 8;
        const unsigned shift = bi % 8;

        return (data.at(byteIndex) >> (shift)) & 1;
    }

    void setTilePriority(unsigned metaTile, unsigned subTile, bool v)
    {
        const unsigned bi = bitIndex(metaTile, subTile);
        const unsigned byteIndex = bi / 8;
        const unsigned shift = bi % 8;

        data.at(byteIndex) = (data.at(byteIndex) & ~(1 << shift)) | (v << shift);
    }

    std::pair<unsigned, uint8_t> indexDataPairToSetTilePriority(unsigned metaTile, unsigned subTile, bool v) const
    {
        const unsigned bi = bitIndex(metaTile, subTile);
        const unsigned byteIndex = bi / 8;
        const unsigned shift = bi % 8;

        return { byteIndex, (data.at(byteIndex) & ~(1 << shift)) | (v << shift) };
    }

    bool operator==(const TilePriorities& o) const
    {
        return data == o.data;
    }
};

struct CrumblingTileChain {
    constexpr static uint16_t NO_THIRD_TRANSITION = UINT16_MAX;

    uint8_t firstTileId = 0;
    uint8_t secondTileId = 0;
    uint8_t thirdTileId = 0;

    uint16_t firstDelay = 300;

    // If this value is NO_THIRD_TRANSITION then there will not be a third transition
    uint16_t secondDelay = 900;

    bool hasThirdTransition() const
    {
        return secondDelay != NO_THIRD_TRANSITION;
    }

    bool operator==(const CrumblingTileChain& o) const
    {
        return firstTileId == o.firstTileId
               && secondTileId == o.secondTileId
               && thirdTileId == o.thirdTileId
               && firstDelay == o.firstDelay
               && secondDelay == o.secondDelay;
    }
};

struct MetaTileTilesetInput {
    static const std::string FILE_EXTENSION;

    idstring name;

    // List of palette names used by the tileset.
    // Will be shown in the map editor.
    // The first palette listed will be the one used to extract the animated tilesset.
    std::vector<idstring> palettes;

    Resources::AnimationFramesInput animationFrames;
    std::array<TileCollisionType, N_METATILES> tileCollisions;
    std::array<idstring, N_METATILES> tileFunctionTables;
    TilePriorities tilePriorities;

    std::array<CrumblingTileChain, N_CRUMBLING_TILE_CHAINS> crumblingTiles;

    grid<uint8_t> scratchpad;

    MetaTileTilesetInput();
    bool validate(ErrorList& err) const;

    bool operator==(const MetaTileTilesetInput& o) const
    {
        return name == o.name
               && palettes == o.palettes
               && animationFrames == o.animationFrames
               && tileCollisions == o.tileCollisions
               && tileFunctionTables == o.tileFunctionTables
               && tilePriorities == o.tilePriorities
               && crumblingTiles == o.crumblingTiles
               && scratchpad == o.scratchpad;
    }
    bool operator!=(const MetaTileTilesetInput& o) const { return !(*this == o); }
};

struct MetaTileTilesetData {
    static const int TILESET_FORMAT_VERSION;

    idstring name;
    std::vector<idstring> palettes;
    std::array<uint8_t, N_METATILES> tileFunctionTables;
    std::array<TileCollisionType, N_METATILES> tileCollisions;
    std::array<CrumblingTileChain, N_CRUMBLING_TILE_CHAINS> crumblingTiles;

    Resources::AnimatedTilesetData animatedTileset;

    MetaTileTilesetData(Resources::AnimatedTilesetData&&);

    usize sourceTileSize() const;

    bool validate(ErrorList& err) const;

    std::vector<uint8_t> exportSnesData() const;

private:
    std::vector<uint8_t> convertTileset() const;
};

std::shared_ptr<const MetaTileTilesetData>
convertTileset(const MetaTileTilesetInput& input,
               const Project::DataStore<Resources::PaletteData>& paletteDataStore,
               const InteractiveTilesData& interactiveTilesData,
               ErrorList& err);
}
}
