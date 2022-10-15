/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "imgui.h"

namespace UnTech::Gui {

class AnimationTimer {
private:
    float _time{ 0 };

    static_assert(std::is_same_v<decltype(_time), decltype(ImGuiIO::DeltaTime)>);

public:
    bool active{ false };
    bool ntscRegion{ true };

public:
    AnimationTimer() = default;

    void reset()
    {
        active = false;
        _time = 0;
    }

    void start()
    {
        reset();
        active = true;
    }

    void stop() { active = false; }
    void playPause() { active = !active; }

    template <typename Function>
    void process(Function onNextFrame)
    {
        if (!active) {
            return;
        }

        _time += ImGui::GetIO().DeltaTime;

        if (_time > 1.0f) {
            _time = 0;
            active = false;
        }

        const float _frameTime = ntscRegion ? 1.0f / 60 : 1.0f / 50;

        while (_time > _frameTime) {
            _time -= _frameTime;

            onNextFrame();
        }
    }
};

class SingleAnimationTimer {
public:
    static constexpr unsigned TICKS_PER_SECOND = 300;
    static constexpr unsigned TICKS_PER_NTSC_FRAME = TICKS_PER_SECOND / 60;
    static constexpr unsigned TICKS_PER_PAL_FRAME = TICKS_PER_SECOND / 50;

private:
    AnimationTimer _frameTimer;
    unsigned _frameCounter{ 0 };
    unsigned _tickCounter{ 0 };

public:
    SingleAnimationTimer()
        : _frameTimer()

    {
    }

    [[nodiscard]] bool isActive() const { return _frameTimer.active; }

    void reset()
    {
        _frameTimer.reset();
        _frameCounter = 0;
        _tickCounter = 0;
    }

    void start()
    {
        reset();
        _frameTimer.start();
    }

    void stop() { _frameTimer.stop(); }
    void playPause() { _frameTimer.playPause(); }

    template <typename Function>
    void process(unsigned animationDelay, Function onNextFrame)
    {
        _frameTimer.process([&] {
            _frameCounter++;

            const unsigned nTicks = _frameTimer.ntscRegion ? TICKS_PER_NTSC_FRAME : TICKS_PER_PAL_FRAME;

            _tickCounter += nTicks;
            if (_tickCounter >= animationDelay) {
                _tickCounter = 0;
                onNextFrame();
            }
        });
    }
};

class DualAnimationTimer {
public:
    static constexpr unsigned TICKS_PER_SECOND = 300;
    static constexpr unsigned TICKS_PER_NTSC_FRAME = TICKS_PER_SECOND / 60;
    static constexpr unsigned TICKS_PER_PAL_FRAME = TICKS_PER_SECOND / 50;

private:
    AnimationTimer _frameTimer;
    unsigned _frameCounter{ 0 };
    unsigned _firstCounter{ 0 };
    unsigned _secondCounter{ 0 };

public:
    DualAnimationTimer()
        : _frameTimer()

    {
    }

    [[nodiscard]] bool isActive() const { return _frameTimer.active; }

    void reset()
    {
        _frameTimer.reset();
        _frameCounter = 0;
        _firstCounter = 0;
        _secondCounter = 0;
    }

    void start()
    {
        reset();
        _frameTimer.start();
    }

    void stop() { _frameTimer.stop(); }
    void playPause() { _frameTimer.playPause(); }

    template <typename FirstFunction, typename SecondFunction>
    void process(unsigned firstAnimationDelay, FirstFunction firstFunction,
                 unsigned secondAnimationDelay, SecondFunction secondFunction)
    {

        _frameTimer.process([&] {
            _frameCounter++;

            const unsigned nTicks = _frameTimer.ntscRegion ? TICKS_PER_NTSC_FRAME : TICKS_PER_PAL_FRAME;

            _firstCounter += nTicks;
            if (_firstCounter >= firstAnimationDelay) {
                _firstCounter = 0;
                firstFunction();
            }

            _secondCounter += nTicks;
            if (_secondCounter >= secondAnimationDelay) {
                _secondCounter = 0;
                secondFunction();
            }
        });
    }
};

}
