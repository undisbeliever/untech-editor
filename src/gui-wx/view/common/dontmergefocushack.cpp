/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "dontmergefocushack.h"
#include <wx/spinctrl.h>

using namespace UnTech::View;

DontMergeFocusHack::DontMergeFocusHack(Controller::BaseController& controller)
    : _controller(controller)
{
}

void DontMergeFocusHack::BindEventRecursive(wxWindow* window)
{
    if (wxSpinCtrl* spin = dynamic_cast<wxSpinCtrl*>(window)) {
        spin->Bind(wxEVT_SET_FOCUS, &DontMergeFocusHack::OnWindowFocus, this);

        spin->SetWindowStyleFlag(wxTE_PROCESS_ENTER);
        spin->Bind(wxEVT_TEXT_ENTER, &DontMergeFocusHack::OnTextEnter, this);
    }
    else if (wxTextCtrl* text = dynamic_cast<wxTextCtrl*>(window)) {
        text->Bind(wxEVT_SET_FOCUS, &DontMergeFocusHack::OnWindowFocus, this);
    }
    else {
        for (wxWindow* child : window->GetChildren()) {
            BindEventRecursive(child);
        }
    }
}

void DontMergeFocusHack::OnWindowFocus(wxFocusEvent& event)
{
    _controller.undoStack().dontMergeNextAction();
    event.Skip();
}

void DontMergeFocusHack::OnTextEnter(wxCommandEvent& event)
{
    _controller.undoStack().dontMergeNextAction();
    event.Skip();
}
