#pragma once

#include "models/sprite-importer/document.h"

#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <memory>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class SpriteImporterWindow;

class SpriteImporterApplication : public Gtk::Application {
protected:
    SpriteImporterApplication();

public:
    static Glib::RefPtr<SpriteImporterApplication> create();

protected:
    void on_startup() override;
    void on_activate() override;

    void on_open(const type_vec_files& files, const Glib::ustring& hint) override;

    void on_menu_new();
    void on_menu_open();
    void on_menu_about();

    void on_window_hide(Gtk::Window* window);

private:
    SpriteImporterWindow* create_window();
    SpriteImporterWindow* empty_window();

    void load_file(const std::string& filename);

private:
    Glib::RefPtr<Gtk::Builder> _uiBuilder;
    static const Glib::ustring _uiInfo;
};
}
}
}
