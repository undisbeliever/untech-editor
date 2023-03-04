/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "previewstate.h"
#include "getters.h"
#include <cassert>
#include <cmath>

namespace UnTech::MetaSprite::Animation {

static unsigned calcTimeToNextFrame(const Animation& ani, const AnimationFrame& aniFrame)
{
    switch (ani.durationFormat) {
    case DurationFormat::FRAME:
        return aniFrame.duration;

    case DurationFormat::TIME:
        return aniFrame.duration << 1;

    case DurationFormat::DISTANCE_HORIZONTAL:
    case DurationFormat::DISTANCE_VERTICAL:
        return aniFrame.duration << 3;
    }

    return 0;
}

void PreviewState::resetState()
{
    resetAnimation();

    animationIndex = INT_MAX;

    velocityFP = point(0, 0);
    positionFP = point(0, 0);
}

void PreviewState::resetAnimation()
{
    running = true;

    overrideNextAnimationIndex = INT_MAX;

    aFrameIndex = 0;

    frameTime = 0;
    displayFrameCount = 0;
}

bool PreviewState::processDisplayFrame(const NamedList<Animation>& animations)
{
    const auto [ani, aniFrame] = getAnimationAndFrame(animations, animationIndex, aFrameIndex);

    running = ani && aniFrame;
    if (!running) {
        return false;
    }

    displayFrameCount++;

    positionFP.x += velocityFP.x;
    positionFP.y += velocityFP.y;

    switch (ani->durationFormat) {
    case DurationFormat::FRAME:
        frameTime += 1;
        break;

    case DurationFormat::TIME:
        frameTime += region == PAL ? 6 : 5;
        break;

    case DurationFormat::DISTANCE_VERTICAL:
        frameTime += abs(velocityFP.y >> (FP_SHIFT - 8));
        break;

    case DurationFormat::DISTANCE_HORIZONTAL:
        frameTime += abs(velocityFP.x >> (FP_SHIFT - 8));
        break;
    }

    if (frameTime >= calcTimeToNextFrame(*ani, *aniFrame)) {
        nextAnimationFrame(*ani, animations);
        return true;
    }

    return false;
}

void PreviewState::nextAnimationFrame(const Animation& ani, const NamedList<Animation>& animations)
{
    running = true;
    frameTime = 0;

    aFrameIndex++;
    if (aFrameIndex >= ani.frames.size()) {
        aFrameIndex = 0;
        frameTime = 0;

        if (overrideNextAnimationIndex < animations.size()) {
            animationIndex = overrideNextAnimationIndex;
            overrideNextAnimationIndex = INT_MAX;
        }
        else if (ani.oneShot) {
            // Ensure the last frame is visible in the animation preview.
            aFrameIndex = ani.frames.size() - 1;
            running = false;
        }
        else if (ani.nextAnimation.isValid()) {
            animationIndex = animations.indexOf(ani.nextAnimation).value_or(INT_MAX);
        }
    }
}

void PreviewState::nextAnimationFrame(const NamedList<Animation>& animations)
{
    const auto ani = getAnimation(animations, animationIndex);

    if (ani) {
        nextAnimationFrame(*ani, animations);
    }
    else {
        running = false;
    }
}

}
