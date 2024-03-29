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
        NTSC,
        PAL
    };

public:
    // NOTE: Indexes may be out of bounds.

    unsigned animationIndex{};

    unsigned overrideNextAnimationIndex{};

    unsigned aFrameIndex{};
    unsigned frameTime{};

    unsigned displayFrameCount{};

    Region region{ Region::NTSC };

    // fixed point
    point velocityFP;
    point positionFP;

    bool running{};

public:
    PreviewState()
    {
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
    [[nodiscard]] point positionInt() const { return { positionFP.x >> FP_SHIFT, positionFP.y >> FP_SHIFT }; }

private:
    void nextAnimationFrame(const Animation& ani, const NamedList<Animation>& animations);
};

}
