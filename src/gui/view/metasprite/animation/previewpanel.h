/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once
#include "gui/controllers/metasprite/animation.h"
#include "models/metasprite/animation/previewstate.h"
#include <wx/tglbtn.h>
#include <wx/wx.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace Animation {

namespace MSA = UnTech::MetaSprite::Animation;

class PreviewPanel : public wxPanel {
    // (1:7:8 fixed point)
    static constexpr int MAX_SLIDER_VELOCITY = 0x02ff;

public:
    PreviewPanel(wxWindow* parent, int wxWindowID,
                 MSA::AnimationControllerInterface& controller);

    void SetTimer();
    void StopTimer();
    void Reset();

private:
    void ProcessDisplayFrame();
    void ProcessSkipFrame();
    void UpdateStatus();

    void OnAnimationSelected();

private:
    MSA::AnimationControllerInterface& _controller;
    MSA::PreviewState _previewState;

    wxPanel* _graphicsPanel;

    wxToggleButton* _playButton;
    wxButton* _stepButton;
    wxButton* _resetButton;
    wxButton* _skipButton;

    wxChoice* _region;
    wxSlider* _xVelocity;
    wxSlider* _yVelocity;

    wxStaticText* _status;

    wxTimer _timer;
};
}
}
}
}
