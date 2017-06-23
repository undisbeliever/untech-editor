/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "durationformat.h"
#include "../common.h"
#include "models/common/capped_vector.h"
#include "models/common/idmap.h"
#include "models/common/idstring.h"
#include <string>
#include <vector>

namespace UnTech {
namespace MetaSprite {
namespace MetaSprite {
struct FrameSet;
}
namespace SpriteImporter {
struct FrameSet;
}

namespace Animation {
struct Animation;

struct AnimationFrame {
    typedef capped_vector<AnimationFrame, MAX_ANIMATION_FRAMES> list_t;

    NameReference frame;
    uint8_t duration;

    bool isValid(const MetaSprite::FrameSet&) const;
    bool isValid(const SpriteImporter::FrameSet&) const;

    bool operator==(const AnimationFrame& o) const
    {
        return this->frame == o.frame && this->duration == o.duration;
    }
    bool operator!=(const AnimationFrame& o) const { return !(*this == o); }
};

struct Animation {
    typedef idmap<Animation> map_t;

    AnimationFrame::list_t frames;
    DurationFormat durationFormat;
    idstring nextAnimation;
    bool oneShot = false;

    bool loopsItself() const
    {
        return !oneShot && !nextAnimation.isValid();
    }

    bool isValid(const MetaSprite::FrameSet&) const;
    bool isValid(const SpriteImporter::FrameSet&) const;

private:
    template <class FrameSetT>
    bool _isValid(const FrameSetT&) const;
};
}
}
}
