/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "previewpanel.h"
#include "gui/controllers/metasprite/metasprite.h"
#include "gui/controllers/metasprite/spriteimporter.h"

using namespace UnTech::View::MetaSprite::Animation;

PreviewPanel::PreviewPanel(wxWindow* parent, int wxWindowID,
                           MSA::AnimationControllerInterface& controller,
                           VS::SettingsController& settingsController,
                           std::unique_ptr<AbstractPreviewRenderer> renderer)
    : wxPanel(parent, wxWindowID)
    , _controller(controller)
    , _settingsController(settingsController)
    , _previewState()
    , _renderer(std::move(renderer))
{
    const int defBorder = wxSizerFlags::GetDefaultBorder();

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer);

    _graphicsPanel = new wxPanel(this, wxID_ANY,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxBORDER_SUNKEN);
    _graphicsPanel->SetDoubleBuffered(true);
    sizer->Add(_graphicsPanel, wxSizerFlags(5).Expand());

    auto* controlsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(controlsSizer, wxSizerFlags().Expand().Border(wxLEFT));

    // Buttons
    {
        auto* grid = new wxGridSizer(2, 2, defBorder, defBorder);
        controlsSizer->Add(grid, wxSizerFlags().Expand().Border());

        _playButton = new wxToggleButton(this, wxID_ANY, L"\u25B6");
        _playButton->SetToolTip("Play");
        grid->Add(_playButton, wxSizerFlags().Expand());

        _stepButton = new wxButton(this, wxID_ANY, L"\u25B7");
        _stepButton->SetToolTip("Step");
        grid->Add(_stepButton, wxSizerFlags().Expand());

        _resetButton = new wxButton(this, wxID_ANY, L"\u23EE");
        _resetButton->SetToolTip("Reset");
        grid->Add(_resetButton, wxSizerFlags().Expand());

        _skipButton = new wxButton(this, wxID_ANY, L"\u23E9");
        _skipButton->SetToolTip("Skip Animation Frame");
        grid->Add(_skipButton, wxSizerFlags().Expand());
    }

    controlsSizer->AddSpacer(defBorder);

    // Settings
    {
        auto* grid = new wxFlexGridSizer(4, 2, defBorder, defBorder * 2);
        controlsSizer->Add(grid, wxSizerFlags(1).Expand().Border());

        grid->AddGrowableCol(1, 1);

        wxArrayString regionChoices;
        regionChoices.Add("NTSC");
        regionChoices.Add("PAL");

        _region = new wxChoice(this, wxID_ANY,
                               wxDefaultPosition, wxDefaultSize, regionChoices);
        _region->SetSelection(0);
        grid->Add(new wxStaticText(this, wxID_ANY, "Region:"));
        grid->Add(_region, wxSizerFlags(1).Expand());

        _xVelocity = new wxSlider(this, wxID_ANY, 0, -MAX_SLIDER_VELOCITY, MAX_SLIDER_VELOCITY);
        grid->Add(new wxStaticText(this, wxID_ANY, "X Velocity:"));
        grid->Add(_xVelocity, wxSizerFlags(1).Expand());

        _yVelocity = new wxSlider(this, wxID_ANY, 0, -MAX_SLIDER_VELOCITY, MAX_SLIDER_VELOCITY);
        grid->Add(new wxStaticText(this, wxID_ANY, "Y Velocity:"));
        grid->Add(_yVelocity, wxSizerFlags(1).Expand());
    }

    controlsSizer->AddSpacer(defBorder);

    _status = new wxStaticText(this, wxID_ANY, wxEmptyString);
    controlsSizer->Add(_status, wxSizerFlags(2).Expand().Top().Border());

    _timer.SetOwner(this, wxID_ANY);

    Reset();

    // Signals
    // -------
    _controller.animationController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &PreviewPanel::OnAnimationSelected));

    // Events
    // ------
    _graphicsPanel->Bind(wxEVT_PAINT, [this](wxPaintEvent&) {
        wxPaintDC dc(_graphicsPanel);
        RenderGraphicsPanel(dc);
    });

    _playButton->Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) {
        if (_playButton->GetValue()) {
            Reset();
            SetTimer();
        }
        else {
            _timer.Stop();
        }
    });

    _stepButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        StopTimer();
        ProcessDisplayFrame();
    });

    _stepButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        StopTimer();
        ProcessDisplayFrame();
    });

    _resetButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        Reset();
    });

    _skipButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        StopTimer();
        ProcessSkipFrame();
    });

    _region->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
        if (_timer.IsRunning()) {
            SetTimer();
        }
    });

    auto onVelocityChange = [this](wxCommandEvent&) {
        point v(_xVelocity->GetValue(), _yVelocity->GetValue());
        _previewState.setVelocityFp(v);
    };
    _xVelocity->Bind(wxEVT_SLIDER, onVelocityChange);
    _yVelocity->Bind(wxEVT_SLIDER, onVelocityChange);

    this->Bind(
        wxEVT_TIMER, [this](wxTimerEvent&) {
            ProcessDisplayFrame();
        },
        _timer.GetId());
}

void PreviewPanel::SetTimer()
{
    if (_previewState.isRunning() == false) {
        StopTimer();
    }

    if (_region->GetSelection() == 0) {
        _timer.Start(1000 / 60);
        _previewState.setRegion(MSA::PreviewState::Region::NTSC);
    }
    else {
        _timer.Start(1000 / 50);
        _previewState.setRegion(MSA::PreviewState::Region::PAL);
    }

    _playButton->SetValue(true);
}

void PreviewPanel::StopTimer()
{
    _timer.Stop();
    _playButton->SetValue(false);

    UpdateGui();
}

void PreviewPanel::Reset()
{
    _previewState.setAnimationMap(_controller.animationController().map());
    _previewState.setAnimation(_controller.animationController().selectedId());
    _previewState.setPositionInt(point(0, 0));
    _previewState.resetFrameCount();

    StopTimer();

    UpdateGui();
}

void PreviewPanel::OnAnimationSelected()
{
    this->Enable(_controller.animationController().hasSelected());

    Reset();
}

void PreviewPanel::ProcessDisplayFrame()
{
    if (!this->IsEnabled() || !this->IsShown()) {
        StopTimer();
        return;
    }

    _previewState.processDisplayFrame();

    UpdateGui();

    if (!_previewState.isRunning()) {
        StopTimer();
    }
}

void PreviewPanel::ProcessSkipFrame()
{
    _previewState.nextAnimationFrame();
    UpdateGui();

    if (!_previewState.isRunning()) {
        StopTimer();
    }
}

void PreviewPanel::UpdateGui()
{
    _graphicsPanel->Refresh();

    wxString out;

    out << "Display Frame:\t" << _previewState.displayFrameCount()
        << "\nMetaSprite Frame:\t" << _previewState.frame().str()
        << "\nAnimation Frame:\t";

    if (_previewState.isRunning()) {
        out << _previewState.animationId().str() << "." << _previewState.animationFrameIndex();
    }

    _status->SetLabel(out);
}

void PreviewPanel::RenderGraphicsPanel(wxPaintDC& paintDc)
{
    if (paintDc.IsOk() == false) {
        return;
    }

    // Ensure preview is inside graphics panel.
    auto checkAxis = [](int posFp, int clientSize, double zoom) {
        int limit = clientSize / zoom * 0.85 / 2.0;
        limit = limit << 8;

        if (posFp > limit) {
            posFp = -limit;
        }
        else if (posFp < -limit) {
            posFp = limit;
        }
        return posFp;
    };
    const auto& zoom = _settingsController.zoom();
    wxSize size = paintDc.GetSize();
    point posFp = _previewState.positionFp();
    posFp.x = checkAxis(posFp.x, size.x, zoom.zoomX());
    posFp.y = checkAxis(posFp.y, size.y, zoom.zoomY());

    _previewState.setPositionFp(posFp);

    _renderer->Render(paintDc, _previewState);
}
