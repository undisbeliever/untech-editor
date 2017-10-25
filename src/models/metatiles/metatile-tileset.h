/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "models/common/idstring.h"
#include "models/resources/animated-tileset.h"
#include "models/resources/animation-frames-input.h"
#include "models/resources/resources.h"
#include <string>
#include <vector>

namespace UnTech {
namespace MetaTiles {

struct MetaTileTilesetInput {
    static const std::string FILE_EXTENSION;

    idstring name;

    // List of palette names used by the tileset.
    // Will be shown in the map editor.
    // The first palette listed will be the one used to extract the animated tilesset.
    std::vector<idstring> palettes;

    Resources::AnimationFramesInput animationFrames;

    void validate() const;
};

struct MetaTileTilesetData {
    static const unsigned TILESET_FORMAT_VERSION;

    idstring name;
    std::vector<idstring> palettes;

    Resources::AnimatedTilesetData animatedTileset;

    // raises an exception if invalid
    void validate(const EngineSettings& settings) const;

    std::vector<uint8_t> exportMetaTileTileset(const EngineSettings& settings) const;

private:
    std::vector<uint8_t> convertTileMap(const EngineSettings& settings) const;
};

MetaTileTilesetData convertTileset(const MetaTileTilesetInput& input,
                                   const Resources::ResourcesFile& resourcesFile);
}
}
