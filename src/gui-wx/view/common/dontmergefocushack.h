/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-wx/controllers/basecontroller.h"
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
    void OnTextEnter(wxCommandEvent&);

private:
    Controller::BaseController& _controller;
};
}
}
