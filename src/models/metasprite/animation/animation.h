/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "durationformat.h"
#include "../common.h"
#include "models/common/idstring.h"
#include "models/common/namedlist.h"
#include <string>
#include <vector>

namespace UnTech {
class ErrorList;
}

namespace UnTech::MetaSprite::MetaSprite {
struct FrameSet;
}
namespace UnTech::MetaSprite::SpriteImporter {
struct FrameSet;
}

namespace UnTech::MetaSprite::Animation {

struct Animation;

struct AnimationFrame {
    NameReference frame;
    uint8_t duration;

    bool testFrameValid(const MetaSprite::FrameSet&) const;
    bool testFrameValid(const SpriteImporter::FrameSet&) const;

    bool operator==(const AnimationFrame& o) const
    {
        return this->frame == o.frame && this->duration == o.duration;
    }
    bool operator!=(const AnimationFrame& o) const { return !(*this == o); }
};

struct Animation {
    idstring name;
    std::vector<AnimationFrame> frames;
    DurationFormat durationFormat;
    idstring nextAnimation;
    bool oneShot = false;

    bool loopsItself() const
    {
        return !oneShot && !nextAnimation.isValid();
    }

    bool operator==(const Animation& o) const
    {
        return frames == o.frames
               && durationFormat == o.durationFormat
               && nextAnimation == o.nextAnimation
               && oneShot == o.oneShot;
    }
    bool operator!=(const Animation& o) const { return !(*this == o); }
};

}
