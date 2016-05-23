#pragma once

#include "gui/widgets/defaults.h"
#include "models/common/namedlist.h"

#include <memory>

#include <glibmm/i18n.h>
#include <gtkmm.h>

namespace UnTech {
namespace Widgets {

class NamedListNameEntry : public Gtk::Entry {
    void on_insert_text(const Glib::ustring& text, int* position) override
    {
        if (text == " ") {
            // Convert spaces to underscore
            Gtk::Entry::on_insert_text("_", position);
        }
        else {
            // allow only id characters
            for (const char c : text) {
                if (!UnTech::isNameCharValid(c)) {
                    return;
                }
            }

            Gtk::Entry::on_insert_text(text, position);
        }
    }
};
}
}
