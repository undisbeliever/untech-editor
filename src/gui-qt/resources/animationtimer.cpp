/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationtimer.h"

#include <QTimerEvent>

using namespace UnTech::GuiQt::Resources;

AnimationTimer::AnimationTimer(QObject* parent)
    : QObject(parent)
    , _nsPerFrame(1000000000 / 60)
    , _ticksPerFrame(TICKS_PER_SECOND / 60)
    , _animationDelay(0)
    , _animationTicksCurrent(0)
    , _playButton(nullptr)
    , _regionCombo(nullptr)
    , _enabled(true)
{
}

void AnimationTimer::timerEvent(QTimerEvent* event)
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

        _animationTicksCurrent += _ticksPerFrame;

        if (_animationTicksCurrent >= _animationDelay) {
            _animationTicksCurrent = 0;

            animationFrameAdvance();
        }
    }
}

void AnimationTimer::setEnabled(bool enabled)
{
    _enabled = enabled;

    if (_playButton) {
        _playButton->setEnabled(_enabled);
    }

    if (enabled == false) {
        stopTimer();
    }
}

void AnimationTimer::setPlayButton(QAbstractButton* button)
{
    Q_ASSERT(_playButton == nullptr);
    Q_ASSERT(button);

    _playButton = button;
    _playButton->setEnabled(_enabled);

    connect(_playButton, &QAbstractButton::clicked,
            this, &AnimationTimer::onPlayButtonClicked);
}

void AnimationTimer::setRegionCombo(QComboBox* region)
{
    Q_ASSERT(_regionCombo == nullptr);
    Q_ASSERT(region);

    region->addItem("NTSC", 1000000000 / 60);
    region->addItem("PAL", 1000000000 / 50);

    _regionCombo = region;
    connect(_regionCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &AnimationTimer::onRegionChanged);

    onRegionChanged();
}

void AnimationTimer::startTimer()
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

    _animationTicksCurrent = 0;

    emit animationStarted();
}

void AnimationTimer::stopTimer()
{
    if (_playButton) {
        _playButton->setChecked(false);
    }

    if (_timer.isActive()) {
        _timer.stop();
        _animationTicksCurrent = 0;

        emit animationStopped();
    }
}

void AnimationTimer::onPlayButtonClicked()
{
    if (_playButton && _enabled && _playButton->isChecked()) {
        startTimer();
    }
    else {
        stopTimer();
    }
}

void AnimationTimer::onRegionChanged()
{
    if (_regionCombo == nullptr) {
        return;
    }

    _nsPerFrame = _regionCombo->currentData().toUInt();
    _ticksPerFrame = (_nsPerFrame == 1000000000 / 60) ? TICKS_PER_SECOND / 60
                                                      : TICKS_PER_SECOND / 50;
}
