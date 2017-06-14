/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "graphicspanel.h"
#include "framegraphicsctrl.h"
#include "tilesetctrl.h"

using namespace UnTech::View::MetaSprite::MetaSprite;

GraphicsPanel::GraphicsPanel(wxWindow* parent, wxWindowID windowId,
                             MS::MetaSpriteController& controller)
    : wxPanel(parent, windowId)
    , _controller(controller)
    , _currentFrameCtrl(0)
    , _split(true)
{
    for (auto& w : _frames) {
        w = new FrameGraphicsCtrl(this, wxID_ANY, _controller);
    }
    _tileset = new TilesetCtrl(this, wxID_ANY, _controller);

    auto* vSizer = new wxBoxSizer(wxVERTICAL);
    auto* hSizer = new wxBoxSizer(wxHORIZONTAL);
    hSizer->Add(_frames[0], wxSizerFlags(1).Expand());
    hSizer->Add(_frames[1], wxSizerFlags(1).Expand());

    vSizer->Add(hSizer, wxSizerFlags(1).Expand());
    vSizer->Add(_tileset, wxSizerFlags(1).Expand().Border(wxTOP));
    this->SetSizer(vSizer);

    SetSplit(false);

    // Signals
    // -------
    _controller.frameController().signal_selectedChanged().connect([this](void) {
        const idstring& frameId = _controller.frameController().selectedId();
        _frames.at(_currentFrameCtrl)->SetCurrentFrameId(frameId);
    });

    // Events
    // ------
    this->Bind(wxEVT_CHILD_FOCUS, [this](wxChildFocusEvent& event) {
        // select the current frame
        for (unsigned i = 0; i < _frames.size(); i++) {
            if (_frames[i] == event.GetEventObject()) {
                _currentFrameCtrl = i;
                _controller.frameController().selectId(_frames[i]->GetCurrentFrameId());
                break;
            }
        }
        event.Skip();
    });
}

void GraphicsPanel::SetSplit(bool split)
{
    if (_split != split) {
        _split = split;

        if (split) {
            for (auto* w : _frames) {
                w->Show();
                w->CenterScrollbar();
            }
        }
        else {
            for (unsigned i = 0; i < _frames.size(); i++) {
                if (i != _currentFrameCtrl) {
                    _frames[i]->Hide();
                }
            }
        }

        if (auto* parent = GetParent()) {
            parent->Layout();
        }
    }
}

void GraphicsPanel::CenterMetaSpriteFrames()
{
    for (auto* w : _frames) {
        w->CenterScrollbar();
    }
}