/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
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

bool AnimationFrame::testFrameValid(const SI::FrameSet& frameSet) const
{
    return frameSet.frames.find(frame.name).exists();
}

bool AnimationFrame::testFrameValid(const MS::FrameSet& frameSet) const
{
    return frameSet.frames.find(frame.name).exists();
}
