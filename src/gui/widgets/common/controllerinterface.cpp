#include "controllerinterface.h"

#include <glibmm/i18n.h>

using namespace UnTech::Widgets;

ControllerInterface::ControllerInterface(Gtk::ApplicationWindow& window)
    : _window(window)
{
}

void ControllerInterface::showError(const char* error, const std::exception& ex)
{
    Gtk::MessageDialog dialog(_(error), false,
                              Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);

    dialog.set_transient_for(_window);

    dialog.set_secondary_text(ex.what());

    dialog.run();
}
