#pragma once

#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <iostream>
#include <stdexcept>

namespace UnTech {
namespace Widgets {

inline void showErrorMessage(Gtk::Window* window, const char* message, const std::exception& error)
{
    std::cerr << message << ": " << error.what() << std::endl;

    Gtk::MessageDialog dialog(_(message), false,
                              Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);

    if (window) {
        dialog.set_transient_for(*window);
    }

    dialog.set_secondary_text(error.what());

    dialog.run();
}

inline void showErrorMessage(Gtk::Widget& parent, const char* message, const std::exception& error)
{
    std::cerr << message << ": " << error.what() << std::endl;

    Gtk::MessageDialog dialog(_(message), false,
                              Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);

    // Set transient parent from widget
    // Could not derefernce RefPtr<Gtk::Window> for base-class constructor
    // using C method instead
    GtkWidget* toplevel = gtk_widget_get_toplevel(parent.gobj());
    gtk_window_set_transient_for(GTK_WINDOW(dialog.gobj()), GTK_WINDOW(toplevel));

    dialog.set_secondary_text(error.what());

    dialog.run();
}
}
}
