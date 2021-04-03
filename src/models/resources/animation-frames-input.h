/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "animated-tileset.h"
#include "palette.h"
#include <filesystem>
#include <vector>

namespace UnTech {
class ErrorList;
namespace Project {
template <typename T>
class DataStore;
}

namespace Resources {

struct PaletteInput;

struct AnimationFramesInput {
    std::vector<std::filesystem::path> frameImageFilenames;

    // Palette used in tileset convertion, may not be the palette used on screen.
    idstring conversionPalette;

    unsigned animationDelay = 30;

    unsigned bitDepth = 4;
    bool addTransparentTile = false;

    bool isBitDepthValid() const;

    bool operator==(const AnimationFramesInput& o) const
    {
        return frameImageFilenames == o.frameImageFilenames
               && conversionPalette == o.conversionPalette
               && animationDelay == o.animationDelay
               && bitDepth == o.bitDepth
               && addTransparentTile == o.addTransparentTile;
    }
    bool operator!=(const AnimationFramesInput& o) const { return !(*this == o); }
};

std::optional<AnimatedTilesetData>
convertAnimationFrames(const AnimationFramesInput& input,
                       const Project::DataStore<PaletteData>& projectDataStore,
                       ErrorList& err);
}
}
