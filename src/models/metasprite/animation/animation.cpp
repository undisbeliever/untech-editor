/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animation.h"
#include "models/metasprite/metasprite.h"
#include "models/metasprite/spriteimporter.h"

using namespace UnTech;
using namespace UnTech::MetaSprite::Animation;

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

// AnimationFrame
// ==============

bool AnimationFrame::isValid(const SI::FrameSet& frameSet) const
{
    return frameSet.frames.contains(frame.name);
}

bool AnimationFrame::isValid(const MS::FrameSet& frameSet) const
{
    return frameSet.frames.contains(frame.name);
}

// Animation
// =========

template <class FrameSetT>
bool Animation::_isValid(const FrameSetT& frameSet) const
{
    if (frameSet.animations.size() > MAX_ANIMATION_FRAMES) {
        return false;
    }

    if (oneShot == false && nextAnimation.isValid()) {
        if (frameSet.animations.contains(nextAnimation) == false) {
            return false;
        }
    }

    if (frames.size() == 0) {
        return false;
    }

    for (const AnimationFrame& aFrame : frames) {
        if (aFrame.isValid(frameSet) == false) {
            return false;
        }
    }

    return true;
}

bool Animation::isValid(const MS::FrameSet& frameSet) const
{
    return _isValid(frameSet);
}

bool Animation::isValid(const SI::FrameSet& frameSet) const
{
    return _isValid(frameSet);
}
