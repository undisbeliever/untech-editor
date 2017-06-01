/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animation-sidebarpage.h"
#include "animation-sidebarlists.hpp"
#include "gui/view/common/enumclasschoice.h"
#include "gui/view/common/idstringtextctrl.h"
#include <wx/spinctrl.h>

namespace UnTech {
namespace View {
namespace MetaSprite {
namespace Animation {

namespace MSA = UnTech::MetaSprite::Animation;

class AnimationPanel : public wxPanel {

public:
    AnimationPanel(wxWindow* parent, int wxWindowID,
                   MSA::AnimationController& controller);

private:
    void UpdateGui();

private:
    MSA::AnimationController& _controller;

    EnumClassChoice<MSA::DurationFormat>* _durationFormat;
    wxCheckBox* _oneShot;
    wxStaticText* _nextAnimationLabel;
    IdStringTextCtrl* _nextAnimation;
};

class AnimationFramePanel : public wxPanel {

public:
    AnimationFramePanel(wxWindow* parent, int wxWindowID,
                        MSA::AnimationFrameController& controller);

private:
    void UpdateGui();

    void OnFrameChanged();

private:
    MSA::AnimationFrameController& _controller;

    IdStringTextCtrl* _frameName;
    wxChoice* _frameFlip;
    wxSpinCtrl* _duration;
    wxStaticText* _durationMeaning;
};
}
}
}
}

using namespace UnTech::View::MetaSprite::Animation;

// ANIMATION SIDEBAR
// =================

AnimationSidebarPage::AnimationSidebarPage(wxWindow* parent, int wxWindowID,
                                           MSA::AnimationControllerInterface& controller)
    : wxPanel(parent, wxWindowID)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer);

    sizer->Add(new IdMapListToolBar<MSA::AnimationController>(
                   this, wxID_ANY,
                   controller.animationController()),
               wxSizerFlags(0).Right().Border());

    sizer->Add(new IdMapListCtrl<MSA::AnimationController>(
                   this, wxID_ANY,
                   controller.animationController()),
               wxSizerFlags(2).Expand().Border(wxLEFT | wxRIGHT));

    // Animation Frame
    auto* aniSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Animation");
    sizer->Add(aniSizer, wxSizerFlags().Expand().Border());

    aniSizer->Add(new AnimationPanel(
                      this, wxID_ANY,
                      controller.animationController()),
                  wxSizerFlags().Expand().Border());

    // Animation Frame
    auto* aFrameSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Animation Frames");
    sizer->Add(aFrameSizer, wxSizerFlags(3).Expand().Border());

    aFrameSizer->Add(new VectorListToolBar<MSA::AnimationFrameController>(
                         this, wxID_ANY,
                         controller.animationFrameController()),
                     wxSizerFlags().Right().Border());

    aFrameSizer->Add(new VectorListCtrl<MSA::AnimationFrameController>(
                         this, wxID_ANY,
                         controller.animationFrameController()),
                     wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT));

    aFrameSizer->Add(new AnimationFramePanel(
                         this, wxID_ANY,
                         controller.animationFrameController()),
                     wxSizerFlags().Expand().Border());
}

// ANIMATION
// =========

AnimationPanel::AnimationPanel(wxWindow* parent, int wxWindowID,
                               MSA::AnimationController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller)
{
    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(3, 2, defBorder, defBorder * 2);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    _durationFormat = new EnumClassChoice<MSA::DurationFormat>(this, wxID_ANY);
    grid->Add(new wxStaticText(this, wxID_ANY, "Duration Format:"));
    grid->Add(_durationFormat, wxSizerFlags(1).Expand());

    _oneShot = new wxCheckBox(this, wxID_ANY, "One Shot");
    grid->Add(_oneShot, wxSizerFlags(1));
    grid->Add(new wxPanel(this, wxID_ANY));

    _nextAnimationLabel = new wxStaticText(this, wxID_ANY, "Next Animation:");
    _nextAnimation = new IdStringTextCtrl(this, wxID_ANY);
    grid->Add(_nextAnimationLabel);
    grid->Add(_nextAnimation, wxSizerFlags(1).Expand());

    // Signals
    // -------
    _controller.signal_anyChanged().connect(sigc::mem_fun(
        *this, &AnimationPanel::UpdateGui));

    // Events
    // ------
    _durationFormat->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
        _controller.selected_setDurationFormat(_durationFormat->GetValue());
    });

    _oneShot->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
        _controller.selected_setOneShot(_oneShot->GetValue());
    });

    _nextAnimation->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&) {
        _controller.selected_setNextAnimation(_nextAnimation->GetValue().ToStdString());
    });
    _nextAnimation->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& e) {
        _controller.selected_setNextAnimation(_nextAnimation->GetValue().ToStdString());
        e.Skip();
    });
}

void AnimationPanel::UpdateGui()
{
    this->Enable(_controller.hasSelected());

    const MSA::Animation& animation = _controller.selected();

    _durationFormat->SetValue(animation.durationFormat);
    _oneShot->SetValue(animation.oneShot);

    _nextAnimationLabel->Enable(animation.oneShot == false);
    _nextAnimation->Enable(animation.oneShot == false);
    _nextAnimation->ChangeValue(animation.nextAnimation.str());
}

// ANIMATION FRAME
// ===============

AnimationFramePanel::AnimationFramePanel(wxWindow* parent, int wxWindowID,
                                         MSA::AnimationFrameController& controller)
    : wxPanel(parent, wxWindowID)
    , _controller(controller)
{
    int defBorder = wxSizerFlags::GetDefaultBorder();
    auto* grid = new wxFlexGridSizer(3, 2, defBorder, defBorder * 2);
    this->SetSizer(grid);

    grid->AddGrowableCol(1, 1);

    wxArrayString flipChoices;
    flipChoices.Add(wxEmptyString);
    flipChoices.Add("hFlip");
    flipChoices.Add("vFlip");
    flipChoices.Add("hvFlip");

    _frameName = new IdStringTextCtrl(this, wxID_ANY,
                                      idstring(), wxDefaultPosition, wxDefaultSize,
                                      wxTE_PROCESS_ENTER);
    _frameFlip = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, flipChoices);

    grid->Add(new wxStaticText(this, wxID_ANY, "Frame:"));
    {
        auto* box = new wxBoxSizer(wxHORIZONTAL);
        box->Add(_frameName, wxSizerFlags(1).Expand());
        box->Add(_frameFlip, wxSizerFlags().Expand().Border(wxLEFT));
        grid->Add(box, wxSizerFlags().Expand());
    }

    _duration = new wxSpinCtrl(this, wxID_ANY);
    _duration->SetRange(0, 255);
    _durationMeaning = new wxStaticText(this, wxID_ANY, wxEmptyString);

    grid->Add(new wxStaticText(this, wxID_ANY, "Duration:"));
    {
        auto* box = new wxBoxSizer(wxHORIZONTAL);
        box->Add(_duration, wxSizerFlags(1).Expand());
        box->Add(_durationMeaning, wxSizerFlags(1).Expand().DoubleBorder(wxLEFT));
        grid->Add(box, wxSizerFlags().Expand());
    }

    // Signals
    // -------
    _controller.signal_anyChanged().connect(sigc::mem_fun(
        *this, &AnimationFramePanel::UpdateGui));

    // Events
    // ------
    _frameName->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&) {
        OnFrameChanged();
    });
    _frameName->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& e) {
        OnFrameChanged();
        e.Skip();
    });

    _frameFlip->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
        OnFrameChanged();
    });

    _duration->Bind(wxEVT_SPINCTRL, [this](wxCommandEvent&) {
        _controller.selected_setDuration(_duration->GetValue());
    });
}

void AnimationFramePanel::UpdateGui()
{
    this->Enable(_controller.hasSelected());

    const MSA::AnimationFrame& aFrame = _controller.selected();
    const MSA::Animation& animation = _controller.parent().selected();

    _frameName->ChangeValue(aFrame.frame.name.str());
    _frameFlip->SetSelection((aFrame.frame.vFlip << 1) | aFrame.frame.hFlip);

    _duration->SetValue(aFrame.duration);

    if (_controller.hasSelected()) {
        _durationMeaning->SetLabel(animation.durationFormat.durationToString(aFrame.duration));
    }
    else {
        _durationMeaning->SetLabel(wxEmptyString);
    }
}

void AnimationFramePanel::OnFrameChanged()
{
    UnTech::MetaSprite::NameReference ref;

    ref.name = _frameName->GetValue().ToStdString();

    int sel = _frameFlip->GetSelection();
    if (sel < 0 || sel > 3) {
        sel = 0;
    }
    ref.hFlip = sel & 0x01;
    ref.vFlip = sel & 0x02;

    _controller.selected_setFrame(ref);
}
