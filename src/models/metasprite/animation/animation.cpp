/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animation.h"
#include "models/metasprite/metasprite.h"
#include "models/metasprite/spriteimporter.h"

namespace UnTech::MetaSprite::Animation {

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

// AnimationFrame
// ==============

// cppcheck-suppress unusedFunction
bool AnimationFrame::testFrameValid(const SI::FrameSet& frameSet) const
{
    return frameSet.frames.contains(frame.name);
}

// cppcheck-suppress unusedFunction
bool AnimationFrame::testFrameValid(const MS::FrameSet& frameSet) const
{
    return frameSet.frames.contains(frame.name);
}

}
