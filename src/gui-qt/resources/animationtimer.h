/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QAbstractButton>
#include <QBasicTimer>
#include <QComboBox>
#include <QElapsedTimer>
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Resources {

class AnimationTimer : public QObject {
    Q_OBJECT

    static constexpr unsigned TICKS_PER_SECOND = 300;

public:
    AnimationTimer(QObject* parent = nullptr);
    ~AnimationTimer() = default;

    virtual void timerEvent(QTimerEvent* event) final;

    unsigned ticksPerFrame() const { return _ticksPerFrame; }
    unsigned nsPerFrame() const { return _nsPerFrame; }

    unsigned animationDelay() const { return _animationDelay; }
    void setAnimationDelay(unsigned animationDelay) { _animationDelay = animationDelay; }

    bool isEnabled() const { return _enabled; }
    void setEnabled(bool enabled);

    void setPlayButton(QAbstractButton* button);
    void setRegionCombo(QComboBox* region);

public slots:
    void startTimer();
    void stopTimer();

private slots:
    void onPlayButtonClicked();
    void onRegionChanged();

signals:
    void animationStarted();
    void animationFrameAdvance();
    void animationStopped();

private:
    QBasicTimer _timer;
    QElapsedTimer _elapsed;
    qint64 _nsSinceLastFrame;

    unsigned _nsPerFrame;
    unsigned _ticksPerFrame;
    unsigned _animationDelay;

    unsigned _animationTicksCurrent;

    QAbstractButton* _playButton;
    QComboBox* _regionCombo;

    bool _enabled;
};
}
}
}
