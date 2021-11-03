/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "animation.h"
#include "models/common/aabb.h"

namespace UnTech::MetaSprite::Animation {

class PreviewState {
public:
    constexpr static unsigned FP_SHIFT = 8;

    enum Region {
        NTSC = 0,
        PAL = 1
    };

public:
    // NOTE: Indexes may be out of bounds.

    unsigned animationIndex;

    unsigned overrideNextAnimationIndex;

    unsigned aFrameIndex;
    unsigned frameTime;

    unsigned displayFrameCount;

    Region region;

    // fixed point
    point velocityFP;
    point positionFP;

    bool running;

public:
    PreviewState()
    {
        region = Region::NTSC;
        resetState();
    }

    void resetState();
    void resetAnimation();

    void setAnimation(const size_t aniIndex);

    // returns true if frame changes
    bool processDisplayFrame(const NamedList<Animation>& animations);

    void nextAnimationFrame(const NamedList<Animation>& animations);

    void setPositionInt(const point& p)
    {
        positionFP.x = p.x << FP_SHIFT;
        positionFP.y = p.y << FP_SHIFT;
    }
    point positionInt() const { return point(positionFP.x >> FP_SHIFT, positionFP.y >> FP_SHIFT); }

private:
    void nextAnimationFrame(const Animation& currentAnimation, const NamedList<Animation>& animations);
};

}
