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
