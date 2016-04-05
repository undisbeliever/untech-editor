#ifndef _UNTECH_GUI_WIDGETS_COMMON_ERRORMESSAGEDIALOG_H
#define _UNTECH_GUI_WIDGETS_COMMON_ERRORMESSAGEDIALOG_H

#include <stdexcept>
#include <iostream>
#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {

inline void showErrorMessage(Gtk::Window* window, const char* message, const std::exception& error)
{
    std::cerr << message << ": " << error.what() << std::endl;

    Gtk::MessageDialog dialog(*window,
                              _(message), false,
                              Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);

    dialog.set_secondary_text(error.what());

    dialog.run();
}
}
}
#endif
