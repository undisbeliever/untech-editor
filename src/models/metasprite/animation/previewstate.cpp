/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "previewstate.h"
#include <cassert>
#include <cmath>

using namespace UnTech;
using namespace UnTech::MetaSprite::Animation;

PreviewState::PreviewState()
    : _animationMap(nullptr)
    , _animationId()
    , _aFrameIndex(0)
    , _frameTime(0)
    , _frame()
    , _displayFrameCount(0)
    , _region(Region::NTSC)
    , _velocity(0, 0)
{
}

const Animation* PreviewState::getAnimation() const
{
    if (_animationMap == nullptr) {
        return nullptr;
    }
    return _animationMap->getPtr(_animationId);
}

const AnimationFrame* PreviewState::getAnimationFrame() const
{
    const Animation* ani = getAnimation();
    if (ani == nullptr) {
        return nullptr;
    }

    if (_aFrameIndex >= ani->frames.size()) {
        return nullptr;
    }
    return &ani->frames.at(_aFrameIndex);
}

bool PreviewState::isRunning() const
{
    const Animation* ani = getAnimation();
    if (ani == nullptr) {
        return false;
    }

    return ani->frames.size() > 0;
}

void PreviewState::setAnimation(const idstring& aniId)
{
    _animationId = aniId;
    _aFrameIndex = 0;
    _frameTime = 0;

    const Animation* ani = getAnimation();
    if (ani && ani->frames.size() > 0) {
        _frame = ani->frames.at(0).frame;
    }
    else {
        _frame = NameReference();
    }
}

bool PreviewState::processDisplayFrame()
{
    if (!isRunning()) {
        return false;
    }

    const Animation* ani = getAnimation();
    assert(ani != nullptr);

    _displayFrameCount++;

    switch (ani->durationFormat) {
    case DurationFormat::Enum::FRAME:
        _frameTime += 1;
        break;

    case DurationFormat::Enum::TIME:
        _frameTime += _region == PAL ? 6 : 5;
        break;

    case DurationFormat::Enum::DISTANCE_VERTICAL:
        _frameTime += abs(_velocity.y >> (FP_SHIFT - 8));
        break;

    case DurationFormat::Enum::DISTANCE_HORIZONTAL:
        _frameTime += abs(_velocity.x >> (FP_SHIFT - 8));
        break;
    }

    if (_frameTime >= calcTimeToNextFrame()) {
        nextAnimationFrame();
        return true;
    }

    return false;
}

void PreviewState::nextAnimationFrame()
{
    if (!isRunning()) {
        return;
    }

    const Animation* ani = getAnimation();
    assert(ani != nullptr);

    _frameTime = 0;

    _aFrameIndex++;
    if (_aFrameIndex < ani->frames.size()) {
        // next frame
        _frame = ani->frames.at(_aFrameIndex).frame;
    }
    else {
        // goto next animation
        if (ani->oneShot) {
            _aFrameIndex = 0;
            _animationId = idstring();
        }
        else if (ani->nextAnimation.isValid()) {
            setAnimation(ani->nextAnimation);
        }
        else {
            setAnimation(_animationId);
        }
    }
}

unsigned PreviewState::calcTimeToNextFrame() const
{
    const Animation* ani = getAnimation();
    const AnimationFrame* aFrame = getAnimationFrame();

    if (ani == nullptr || aFrame == nullptr) {
        return 0;
    }

    switch (ani->durationFormat.value()) {
    case DurationFormat::Enum::FRAME:
        return aFrame->duration;

    case DurationFormat::Enum::TIME:
        return aFrame->duration << 1;

    case DurationFormat::Enum::DISTANCE_HORIZONTAL:
    case DurationFormat::Enum::DISTANCE_VERTICAL:
        return aFrame->duration << 3;
    }

    return 0;
}
