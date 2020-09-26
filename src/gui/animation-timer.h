/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "imgui.h"

namespace UnTech::Gui {

class AnimationTimer {
private:
    float _time;

    static_assert(std::is_same_v<decltype(_time), decltype(ImGuiIO::DeltaTime)>);

public:
    bool active;
    bool ntscRegion;

public:
    AnimationTimer()
        : _time(0)
        , active(false)
        , ntscRegion(true)
    {
    }

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

}
