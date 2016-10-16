#pragma once

#include "gui/controllers/basecontroller.h"
#include <wx/wx.h>

namespace UnTech {
namespace View {

class DontMergeFocusHack {
public:
    DontMergeFocusHack(Controller::BaseController& controller);
    DontMergeFocusHack() = delete;
    DontMergeFocusHack(const DontMergeFocusHack&) = delete;

    virtual ~DontMergeFocusHack() = default;

    void BindEventRecursive(wxWindow* window);

protected:
    void OnWindowFocus(wxFocusEvent&);

private:
    Controller::BaseController& _controller;
};
}
}
