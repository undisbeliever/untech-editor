/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "previewstate.h"
#include <cassert>
#include <cmath>

using namespace UnTech;
using namespace UnTech::MetaSprite::Animation;

PreviewState::PreviewState()
    : _animations(nullptr)
    , _animationIndex(INT_MAX)
    , _nextAnimationIndex(INT_MAX)
    , _aFrameIndex(0)
    , _frameTime(0)
    , _displayFrameCount(0)
    , _region(Region::NTSC)
    , _velocity(0, 0)
    , _position(0, 0)
{
}

const Animation* PreviewState::getAnimation(size_t index) const
{
    if (_animations) {
        if (index < _animations->size()) {
            return &_animations->at(index);
        }
    }
    return nullptr;
}

const Animation* PreviewState::getAnimation() const
{
    return getAnimation(_animationIndex);
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

const MetaSprite::NameReference& PreviewState::frame() const
{
    const static NameReference BLANK_FRAME = {};

    const Animation* ani = getAnimation();
    if (ani == nullptr || _aFrameIndex >= ani->frames.size()) {
        return BLANK_FRAME;
    }
    else {
        return ani->frames.at(_aFrameIndex).frame;
    }
}

bool PreviewState::isRunning() const
{
    const Animation* ani = getAnimation();
    if (ani == nullptr) {
        return false;
    }

    return ani->frames.size() > 0;
}

const idstring& PreviewState::animationId() const
{
    const static idstring BLANK_ID;

    if (auto* ani = getAnimation()) {
        return ani->name;
    }
    else {
        return BLANK_ID;
    }
}

const idstring& PreviewState::nextAnimationId() const
{
    const static idstring BLANK_ID;

    if (auto* ani = getAnimation(_nextAnimationIndex)) {
        return ani->name;
    }
    else {
        return BLANK_ID;
    }
}

void PreviewState::setAnimationIndex(size_t index)
{
    assert(_animations);

    _animationIndex = index;
    _nextAnimationIndex = INT_MAX;
    _aFrameIndex = 0;
    _frameTime = 0;

    if (auto* ani = getAnimation(_animationIndex)) {
        if (ani->oneShot) {
            _nextAnimationIndex = INT_MAX;
        }
        else if (ani->nextAnimation.isValid()) {
            _nextAnimationIndex = _animations->indexOf(ani->nextAnimation);
        }
        else {
            _nextAnimationIndex = _animationIndex;
        }
    }
}

void PreviewState::setNextAnimationIndex(size_t index)
{
    _nextAnimationIndex = index;
}

bool PreviewState::processDisplayFrame()
{
    if (!isRunning()) {
        return false;
    }

    const Animation* ani = getAnimation();
    assert(ani != nullptr);

    _displayFrameCount++;

    _position.x += _velocity.x;
    _position.y += _velocity.y;

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

    assert(_animations);
    const Animation* ani = getAnimation();
    assert(ani);

    _frameTime = 0;

    _aFrameIndex++;
    if (_aFrameIndex >= ani->frames.size()) {
        if (_nextAnimationIndex < _animations->size()) {
            setAnimationIndex(_nextAnimationIndex);
        }
        else {
            // untech-engine does not do this.
            // This is required for `frame()` to work after the animation ends.
            _aFrameIndex = ani->frames.size() - 1;
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
