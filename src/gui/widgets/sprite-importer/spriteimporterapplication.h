#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SPRITEIMPORTERAPPLICATION_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SPRITEIMPORTERAPPLICATION_H_

#include "document.h"

#include <memory>
#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

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
    void create_window(std::unique_ptr<Document> document);
    void load_file(const std::string& filename);

private:
    Glib::RefPtr<Gtk::Builder> _uiBuilder;
    static const Glib::ustring _uiInfo;
};
}
}
}

#endif
