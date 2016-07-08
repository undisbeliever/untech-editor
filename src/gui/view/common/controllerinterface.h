#pragma once

#include "gui/controllers/basecontroller.h"
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
