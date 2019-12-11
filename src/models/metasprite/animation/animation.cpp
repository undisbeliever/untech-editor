/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animation.h"
#include "models/metasprite/errorlisthelpers.h"
#include "models/metasprite/metasprite.h"
#include "models/metasprite/spriteimporter.h"

using namespace UnTech;
using namespace UnTech::MetaSprite::Animation;

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

// AnimationFrame
// ==============

bool AnimationFrame::testFrameValid(const SI::FrameSet& frameSet) const
{
    return frameSet.frames.find(frame.name).exists();
}

bool AnimationFrame::testFrameValid(const MS::FrameSet& frameSet) const
{
    return frameSet.frames.find(frame.name).exists();
}

// Animation
// =========

template <class FrameSetT>
bool Animation::_validate(const FrameSetT& frameSet, ErrorList& err) const
{
    const unsigned oldErrorCount = err.errorCount();

    if (oneShot == false && nextAnimation.isValid()) {
        if (!frameSet.animations.find(nextAnimation)) {
            err.addError(animationError(*this, "Cannot find animation ", nextAnimation));
        }
    }

    if (frames.size() == 0) {
        err.addError(animationError(*this, "Expected at least one animation frame"));
    }

    for (unsigned i = 0; i < frames.size(); i++) {
        const AnimationFrame& aFrame = frames.at(i);

        if (aFrame.testFrameValid(frameSet) == false) {
            err.addError(animationFrameError(*this, i, "Cannot find frame ", aFrame.frame.name));
        }
    }

    return err.errorCount() == oldErrorCount;
}

bool Animation::validate(const MS::FrameSet& frameSet, ErrorList& err) const
{
    return _validate(frameSet, err);
}

bool Animation::validate(const SI::FrameSet& frameSet, ErrorList& err) const
{
    return _validate(frameSet, err);
}
