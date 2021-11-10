/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animation.h"
#include "models/common/iterators.h"
#include "models/metasprite/errorlisthelpers.h"

namespace UnTech::MetaSprite::Animation {

template <class FrameSetT>
bool validate(const Animation& input, const unsigned aniIndex, const FrameSetT& frameSet, ErrorList& err)
{
    const unsigned oldErrorCount = err.errorCount();

    if (input.oneShot == false && input.nextAnimation.isValid()) {
        if (!frameSet.animations.find(input.nextAnimation)) {
            err.addError(animationError(input, aniIndex, u8"Cannot find animation ", input.nextAnimation));
        }
    }

    if (input.frames.size() == 0) {
        err.addError(animationError(input, aniIndex, u8"Expected at least one animation frame"));
    }

    for (auto [i, aFrame] : const_enumerate(input.frames)) {
        if (aFrame.testFrameValid(frameSet) == false) {
            err.addError(animationFrameError(input, aniIndex, i, u8"Cannot find frame ", aFrame.frame.name));
        }
    }

    return err.errorCount() == oldErrorCount;
}

}
