/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-wx/controllers/basecontroller.h"
#include <stdexcept>
#include <wx/wx.h>

namespace UnTech {
namespace View {

class ControllerInterface : public Controller::ControllerInterface {
public:
    ControllerInterface(wxFrame* frame);
    ControllerInterface() = delete;
    ControllerInterface(const ControllerInterface&) = delete;

    virtual ~ControllerInterface() = default;

    virtual void showError(const char* error, const std::exception& ex) override;

private:
    wxFrame* _frame;
};
}
}
