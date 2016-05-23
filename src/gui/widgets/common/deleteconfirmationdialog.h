#pragma once

#include <glibmm/i18n.h>
#include <gtkmm/messagedialog.h>

namespace UnTech {
namespace Widgets {

class DeleteConfirmationDialog : public Gtk::MessageDialog {
public:
    DeleteConfirmationDialog(const Glib::ustring& name, Gtk::Widget& parent, bool modal = true);

    ~DeleteConfirmationDialog() = default;
};
}
}
