/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "animated-tileset.h"
#include "palette.h"
#include <vector>

namespace UnTech {
class ErrorList;

namespace Resources {

struct PaletteInput;

struct AnimationFramesInput {
    std::vector<std::string> frameImageFilenames;

    unsigned animationDelay = 30;

    unsigned bitDepth = 4;
    bool addTransparentTile = false;

    bool validate(ErrorList& err) const;

    bool operator==(const AnimationFramesInput& o) const
    {
        return frameImageFilenames == o.frameImageFilenames
               && animationDelay == o.animationDelay
               && bitDepth == o.bitDepth
               && addTransparentTile == o.addTransparentTile;
    }
    bool operator!=(const AnimationFramesInput& o) const { return !(*this == o); }
};

// returns nullptr if AnimationFramesInput or AnimatedTilesetData is invalid
std::unique_ptr<AnimatedTilesetData>
convertAnimationFrames(const AnimationFramesInput& input, const PaletteInput& paletteInput,
                       ErrorList& err);
}
}
