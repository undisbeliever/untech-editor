/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "animation.h"
#include "models/common/aabb.h"

namespace UnTech {
namespace MetaSprite {
namespace Animation {

class PreviewState {
public:
    constexpr static unsigned FP_SHIFT = 8;

    enum Region {
        NTSC = 0,
        PAL = 1
    };

public:
    PreviewState();

    void setAnimationMap(const Animation::map_t* map) { _animationMap = map; }
    void setAnimation(const idstring&);

    void setRegion(Region region) { _region = region; }

    void setVelocityFp(const point& v) { _velocity = v; }

    void setPositionFp(const point& p) { _position = p; }
    void setPositionInt(const point& p) { _position = point(p.x << FP_SHIFT, p.y << FP_SHIFT); }

    void resetFrameCount() { _displayFrameCount = 0; }

    // return true if frame changes
    bool processDisplayFrame();
    void nextAnimationFrame();

    const NameReference& frame() const;
    bool isRunning() const;
    const idstring& animationId() const { return _animationId; }
    unsigned animationFrameIndex() const { return _aFrameIndex; }
    unsigned displayFrameCount() const { return _displayFrameCount; }

    const point& positionFp() const { return _position; }
    point positionInt() const { return point(_position.x >> FP_SHIFT, _position.y >> FP_SHIFT); }

private:
    const Animation* getAnimation() const;
    const AnimationFrame* getAnimationFrame() const;

    unsigned calcTimeToNextFrame() const;

private:
    const Animation::map_t* _animationMap;
    idstring _animationId;
    unsigned _aFrameIndex;
    unsigned _frameTime;

    unsigned _displayFrameCount;

    Region _region;

    // fixed point
    point _velocity;
    point _position;
};
}
}
}
