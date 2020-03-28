/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "dualanimationtimer.h"

#include <QTimerEvent>

using namespace UnTech::GuiQt::Resources;

DualAnimationTimer::DualAnimationTimer(QObject* parent)
    : QObject(parent)
    , _nsPerFrame(1000000000 / 60)
    , _ticksPerFrame(TICKS_PER_SECOND / 60)
    , _animationDelay(0, 0)
    , _animationTicksCurrent(0, 0)
    , _animationFrameCount(0, 0)
    , _playButton(nullptr)
    , _regionCombo(nullptr)
    , _enabled(true)
{
}

void DualAnimationTimer::timerEvent(QTimerEvent* event)
{
    if (event->timerId() != _timer.timerId()) {
        return;
    }

    qint64 nsecsElapsed = _elapsed.nsecsElapsed();
    qint64 ns = nsecsElapsed - _nsSinceLastFrame;

    if (ns > _nsPerFrame * 8) {
        // stop if execution if it takes too long
        stopTimer();
    }
    while (ns >= _nsPerFrame) {
        ns -= _nsPerFrame;
        _nsSinceLastFrame += _nsPerFrame;

        auto process = [this](const bool i) {
            auto get = [](std::pair<unsigned, unsigned>& pair, const bool i) -> unsigned& {
                return i ? pair.first : pair.second;
            };

            auto& animationTicksCurrent = get(_animationTicksCurrent, i);
            auto& animationDelay = get(_animationDelay, i);
            auto& animationFrameCount = get(_animationFrameCount, i);

            animationTicksCurrent += _ticksPerFrame;

            if (animationTicksCurrent >= animationDelay) {
                animationTicksCurrent = 0;
                animationFrameCount++;

                return true;
            }
            else {
                return false;
            }
        };

        bool frameCountChanged = process(0) | process(1);
        if (frameCountChanged) {
            emit animationFrameCountChanged();
        }
    }
}

void DualAnimationTimer::pauseAndIncrementFirstFrameCount()
{
    stopTimer();

    _animationFrameCount.first++;
    _animationTicksCurrent.first = 0;

    emit animationFrameCountChanged();
}

void DualAnimationTimer::pauseAndIncrementSecondFrameCount()
{
    stopTimer();

    _animationFrameCount.second++;
    _animationTicksCurrent.second = 0;

    emit animationFrameCountChanged();
}

void DualAnimationTimer::setAnimationDelays(unsigned delay0, unsigned delay1)
{
    _animationDelay.first = delay0;
    _animationDelay.second = delay1;
}

void DualAnimationTimer::setEnabled(bool enabled)
{
    _enabled = enabled;

    if (_playButton) {
        _playButton->setEnabled(_enabled);
    }

    if (enabled == false) {
        stopTimer();
    }
}

void DualAnimationTimer::setPlayButton(QAbstractButton* button)
{
    Q_ASSERT(_playButton == nullptr);
    Q_ASSERT(button);

    _playButton = button;
    _playButton->setEnabled(_enabled);

    connect(_playButton, &QAbstractButton::clicked,
            this, &DualAnimationTimer::onPlayButtonClicked);
}

void DualAnimationTimer::setRegionCombo(QComboBox* region)
{
    Q_ASSERT(_regionCombo == nullptr);
    Q_ASSERT(region);

    region->addItem("NTSC", 1000000000 / 60);
    region->addItem("PAL", 1000000000 / 50);

    _regionCombo = region;
    connect(_regionCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &DualAnimationTimer::onRegionChanged);

    onRegionChanged();
}

void DualAnimationTimer::startTimer()
{
    if (_enabled == false) {
        return;
    }

    if (_playButton) {
        _playButton->setChecked(true);
    }

    _timer.start(1, Qt::PreciseTimer, this);
    _elapsed.restart();
    _nsSinceLastFrame = _elapsed.nsecsElapsed();

    _animationTicksCurrent = { 0, 0 };

    emit animationStarted();
}

void DualAnimationTimer::stopTimer()
{
    if (_playButton) {
        _playButton->setChecked(false);
    }

    if (_timer.isActive()) {
        _timer.stop();
        _animationTicksCurrent = { 0, 0 };

        emit animationStopped();
    }
}

void DualAnimationTimer::resetTimer()
{
    stopTimer();
    resetAnimationFrameCount();
}

void DualAnimationTimer::resetAnimationFrameCount()
{
    _animationFrameCount = { 0, 0 };

    emit animationFrameCountChanged();
}

void DualAnimationTimer::onPlayButtonClicked()
{
    if (_playButton && _enabled && _playButton->isChecked()) {
        startTimer();
    }
    else {
        stopTimer();
    }
}

void DualAnimationTimer::onRegionChanged()
{
    if (_regionCombo == nullptr) {
        return;
    }

    _nsPerFrame = _regionCombo->currentData().toUInt();
    _ticksPerFrame = (_nsPerFrame == 1000000000 / 60) ? TICKS_PER_SECOND / 60
                                                      : TICKS_PER_SECOND / 50;
}
