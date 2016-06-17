#pragma once

#include "gui/controllers/basecontroller.h"
#include <gtkmm.h>
#include <stdexcept>

namespace UnTech {
namespace Widgets {

class ControllerInterface : public Controller::ControllerInterface {
public:
    ControllerInterface(Gtk::ApplicationWindow&);
    ControllerInterface() = delete;
    ControllerInterface(const ControllerInterface&) = delete;

    virtual ~ControllerInterface() = default;

    virtual void showError(const char* error, const std::exception& ex) override;

private:
    Gtk::ApplicationWindow& _window;
};
}
}
