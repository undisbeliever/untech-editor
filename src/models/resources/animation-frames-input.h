/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "animated-tileset.h"
#include "error-list.h"
#include "palette.h"
#include "models/common/image.h"
#include <vector>

namespace UnTech {
namespace Resources {

struct PaletteInput;

struct AnimationFramesInput {
    std::vector<Image> frameImages;
    std::vector<std::string> frameImageFilenames;

    unsigned animationDelay = 30;

    unsigned bitDepth = 4;
    bool addTransparentTile = false;

    bool validate(ErrorList& err) const;
};

// returns nullptr if AnimationFramesInput or AnimatedTilesetData is invalid
std::unique_ptr<AnimatedTilesetData>
convertAnimationFrames(const AnimationFramesInput& input, const PaletteInput& paletteInput,
                       ErrorList& err);
}
}
