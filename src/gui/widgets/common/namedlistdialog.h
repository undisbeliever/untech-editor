#ifndef _UNTECH_GUI_WIDGETS_COMMON_NAMEDLISTDIALOG_H
#define _UNTECH_GUI_WIDGETS_COMMON_NAMEDLISTDIALOG_H

#include "namedlistnameentry.h"
#include "models/common/namedlist.h"
#include "gui/widgets/defaults.h"

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {

template <class T>
class NamedListDialog : public Gtk::Dialog {
public:
    NamedListDialog(const Glib::ustring& title, Gtk::Widget& parent, bool modal = true)
        : Gtk::Dialog(title, modal)
        , _label(title, Gtk::ALIGN_START)
        , _errorLabel()
        , _inputText()
        , _list(nullptr)
        , _item(nullptr)
    {
        set_default_response(Gtk::RESPONSE_ACCEPT);

        _okButton = add_button(_("Apply"), Gtk::RESPONSE_ACCEPT);
        add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);

        get_content_area()->add(_label);
        get_content_area()->add(_inputText);
        get_content_area()->add(_errorLabel);

        get_content_area()->show_all_children();

        on_text_changed();

        // Set transient parent from widget
        // Could not derefernce RefPtr<Gtk::Window> for base-class constructor
        // using C method instead
        GtkWidget* toplevel = gtk_widget_get_toplevel(parent.gobj());
        gtk_window_set_transient_for(GTK_WINDOW(this->gobj()), GTK_WINDOW(toplevel));

        /*
         * SLOTS
         */
        _inputText.signal_changed().connect(sigc::mem_fun(*this, &NamedListDialog::on_text_changed));
        _inputText.signal_activate().connect([this](void) {
            response(Gtk::RESPONSE_ACCEPT);
        });
    }

    ~NamedListDialog() {}

    void setList(typename T::list_t* list)
    {
        _list = list;
        _item = nullptr;
        set_text("");
    }

    void setItem(typename T::list_t* list, T* item)
    {
        _list = list;
        _item = item;

        if (_list && _item) {
            set_text(_list->getName(_item).first);
        }
    }

    Glib::ustring get_text()
    {
        return _inputText.get_text();
    }

    void set_text(const Glib::ustring& text)
    {
        _inputText.set_text(text);
        on_text_changed();
    }

    void set_label(const Glib::ustring& text)
    {
        _inputText.set_text(text);
    }

    bool check_text()
    {
        if (_list) {
            auto text = get_text();

            if (_item) {
                if (_list->getName(_item).first == text) {
                    return true;
                }
            }
            return (isNameValid(text) && _list->nameExists(text));
        }
    }

protected:
    void on_text_changed()
    {
        if (_list) {
            auto text = get_text();

            if (text.empty()) {
                _errorLabel.set_text("Error: Empty String");
                _inputText.set_icon_from_icon_name("dialog-error", Gtk::ENTRY_ICON_SECONDARY);
                _okButton->set_sensitive(false);
                return;
            }

            if (!isNameValid(text)) {
                _errorLabel.set_text("Error: Invalid characters");
                _inputText.set_icon_from_icon_name("dialog-error", Gtk::ENTRY_ICON_SECONDARY);
                _okButton->set_sensitive(false);
                return;
            }

            if (_list->nameExists(text)) {
                if (_item == nullptr || _list->getName(_item).first != text) {
                    _errorLabel.set_text("Error: Name already exists");
                    _inputText.set_icon_from_icon_name("dialog-error", Gtk::ENTRY_ICON_SECONDARY);
                    _okButton->set_sensitive(false);
                    return;
                }
            }

            _errorLabel.set_text("");
            _inputText.unset_icon(Gtk::ENTRY_ICON_SECONDARY);
            _okButton->set_sensitive(true);
            return;
        }
    }

private:
    Gtk::Label _label, _errorLabel;
    NamedListNameEntry _inputText;

    typename T::list_t* _list;
    T* _item;

    Gtk::Button* _okButton;
};
}
}
#endif
