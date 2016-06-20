#pragma once

#include "models/metasprite/document.h"

#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <memory>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class MetaSpriteWindow;

class MetaSpriteApplication : public Gtk::Application {
protected:
    MetaSpriteApplication();

public:
    static Glib::RefPtr<MetaSpriteApplication> create();

protected:
    void on_startup() override;
    void on_activate() override;

    void on_open(const type_vec_files& files, const Glib::ustring& hint) override;

    void on_menu_new();
    void on_menu_open();
    void on_menu_about();

    void on_window_hide(Gtk::Window* window);

private:
    MetaSpriteWindow* create_window();
    MetaSpriteWindow* empty_window();

    void load_file(const std::string& filename);

private:
    Glib::RefPtr<Gtk::Builder> _uiBuilder;
    static const Glib::ustring _uiInfo;
};
}
}
}
