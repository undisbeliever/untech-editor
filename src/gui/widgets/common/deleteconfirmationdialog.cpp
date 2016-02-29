#include "deleteconfirmationdialog.h"

using namespace UnTech::Widgets;

DeleteConfirmationDialog::DeleteConfirmationDialog(const Glib::ustring& name, Gtk::Widget& parent, bool modal)
    : Gtk::MessageDialog(
          Glib::ustring::compose(_("Are you sure you wish to delete %1?"), name),
          false,
          Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE,
          modal)
{
    set_default_response(Gtk::RESPONSE_ACCEPT);

    add_button(_("_Delete"), Gtk::RESPONSE_ACCEPT);
    add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);

    // Set transient parent from widget
    // Could not derefernce RefPtr<Gtk::Window> for base-class constructor
    // using C method instead
    GtkWidget* toplevel = gtk_widget_get_toplevel(parent.gobj());
    gtk_window_set_transient_for(GTK_WINDOW(this->gobj()), GTK_WINDOW(toplevel));
}
