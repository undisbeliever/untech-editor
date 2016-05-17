#ifndef _UNTECH_GUI_WIDGETS_COMMON_DELETECONFIRATIONDIALOG_H
#define _UNTECH_GUI_WIDGETS_COMMON_DELETECONFIRATIONDIALOG_H

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
#endif
