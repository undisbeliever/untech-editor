/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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
namespace Resources {
struct ResourcesFile;
}

namespace MetaTiles {

struct MetaTileTilesetInput {
    static const std::string FILE_EXTENSION;

    idstring name;

    // List of palette names used by the tileset.
    // Will be shown in the map editor.
    // The first palette listed will be the one used to extract the animated tilesset.
    std::vector<idstring> palettes;

    Resources::AnimationFramesInput animationFrames;

    grid<uint16_t> scratchpad;

    bool validate(ErrorList& err) const;

    bool operator==(const MetaTileTilesetInput& o) const
    {
        return name == o.name
               && palettes == o.palettes
               && animationFrames == o.animationFrames;
    }
    bool operator!=(const MetaTileTilesetInput& o) const { return !(*this == o); }
};

struct MetaTileTilesetData {
    static const int TILESET_FORMAT_VERSION;

    idstring name;
    std::vector<idstring> palettes;
    std::unique_ptr<Resources::AnimatedTilesetData> animatedTileset;

    usize sourceTileSize() const;

    unsigned nMetaTiles() const;

    bool validate(const EngineSettings& settings, ErrorList& err) const;

    std::vector<uint8_t> exportMetaTileTileset(const EngineSettings& settings) const;

private:
    std::vector<uint8_t> convertTileMap(const EngineSettings& settings) const;
};

std::unique_ptr<MetaTileTilesetData> convertTileset(const MetaTileTilesetInput& input,
                                                    const Resources::ResourcesFile& resourcesFile,
                                                    ErrorList& err);
}
}
