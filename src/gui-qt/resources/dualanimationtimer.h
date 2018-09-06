/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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

class DualAnimationTimer : public QObject {
    Q_OBJECT

    static constexpr unsigned TICKS_PER_SECOND = 300;

public:
    DualAnimationTimer(QObject* parent = nullptr);
    ~DualAnimationTimer() = default;

    virtual void timerEvent(QTimerEvent* event) final;

    unsigned ticksPerFrame() const { return _ticksPerFrame; }
    unsigned nsPerFrame() const { return _nsPerFrame; }

    void setAnimationDelays(unsigned delay0, unsigned delay1);

    const auto& animationFrameCount() const { return _animationFrameCount; }

    bool isEnabled() const { return _enabled; }
    void setEnabled(bool enabled);

    void setPlayButton(QAbstractButton* button);
    void setRegionCombo(QComboBox* region);

public slots:
    void startTimer();
    void stopTimer();
    void resetTimer();
    void resetAnimationFrameCount();

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

    std::pair<unsigned, unsigned> _animationDelay;
    std::pair<unsigned, unsigned> _animationTicksCurrent;
    std::pair<unsigned, unsigned> _animationFrameCount;

    QAbstractButton* _playButton;
    QComboBox* _regionCombo;

    bool _enabled;
};
}
}
}
