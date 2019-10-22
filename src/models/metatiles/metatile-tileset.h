/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
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
class ErrorList;
namespace Project {
struct ProjectFile;
}

namespace MetaTiles {

// ::KUDOS Christopher Hebert for the slope names::
// ::: Reconstructing Cave Story: Slope Theory::
// ::: https://www.youtube.com/watch?v=ny14i0GxGZw ::
enum class TileCollision : uint8_t {
    EMPTY,
    SOLID,
    UP_PLATFORM,
    DOWN_PLATFORM,
    DOWN_RIGHT_SLOPE, // right and down sides are the biggest, fall down to collide, walk right to ascend
    DOWN_LEFT_SLOPE,
    DOWN_RIGHT_SHORT_SLOPE,
    DOWN_RIGHT_TALL_SLOPE,
    DOWN_LEFT_TALL_SLOPE,
    DOWN_LEFT_SHORT_SLOPE,
    UP_RIGHT_SLOPE,
    UP_LEFT_SLOPE,
    UP_RIGHT_SHORT_SLOPE,
    UP_RIGHT_TALL_SLOPE,
    UP_LEFT_TALL_SLOPE,
    UP_LEFT_SHORT_SLOPE,

    END_SLOPE,
};
constexpr uint8_t N_TILE_COLLISONS = 17;

struct MetaTileTilesetInput {
    static const std::string FILE_EXTENSION;

    idstring name;

    // List of palette names used by the tileset.
    // Will be shown in the map editor.
    // The first palette listed will be the one used to extract the animated tilesset.
    std::vector<idstring> palettes;

    Resources::AnimationFramesInput animationFrames;

    std::array<TileCollision, N_METATILES> tileCollisions{};

    grid<uint8_t> scratchpad;

    bool validate(ErrorList& err) const;

    bool operator==(const MetaTileTilesetInput& o) const
    {
        return name == o.name
               && palettes == o.palettes
               && animationFrames == o.animationFrames
               && scratchpad == o.scratchpad;
    }
    bool operator!=(const MetaTileTilesetInput& o) const { return !(*this == o); }
};

struct MetaTileTilesetData {
    static const int TILESET_FORMAT_VERSION;

    idstring name;
    std::vector<idstring> palettes;
    std::unique_ptr<Resources::AnimatedTilesetData> animatedTileset;
    std::array<TileCollision, N_METATILES> tileCollisions;

    usize sourceTileSize() const;

    bool validate(ErrorList& err) const;

    std::vector<uint8_t> exportMetaTileTileset() const;

private:
    std::vector<uint8_t> convertTileMap() const;
};

std::unique_ptr<MetaTileTilesetData> convertTileset(const MetaTileTilesetInput& input,
                                                    const Project::ProjectFile& projectFile,
                                                    ErrorList& err);
}
}
