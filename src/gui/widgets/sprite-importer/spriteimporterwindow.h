#pragma once

#include "spriteimportereditor.h"
#include "gui/controllers/sprite-importer.h"

#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <memory>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class SpriteImporterWindow : public Gtk::ApplicationWindow {
public:
    SpriteImporterWindow();

    auto& controller() { return _controller; }

protected:
    void updateTitle();
    void updateUndoActions();
    void updateItemActions();

    void updateGuiZoom();

    void do_undo();
    void do_redo();
    void do_save();
    void do_saveAs();

    void do_setZoom(int zoom);
    void do_setAspectRatio(int state);

    bool on_delete_event(GdkEventAny* any_event);

private:
    SI::SpriteImporterController _controller;

    SpriteImporterEditor _editor;

    Glib::RefPtr<Gio::SimpleAction> _saveAction;
    Glib::RefPtr<Gio::SimpleAction> _undoAction;
    Glib::RefPtr<Gio::SimpleAction> _redoAction;

    Glib::RefPtr<Gio::SimpleAction> _createSelectedAction;
    Glib::RefPtr<Gio::SimpleAction> _cloneSelectedAction;
    Glib::RefPtr<Gio::SimpleAction> _removeSelectedAction;
    Glib::RefPtr<Gio::SimpleAction> _moveSelectedUpAction;
    Glib::RefPtr<Gio::SimpleAction> _moveSelectedDownAction;
    Glib::RefPtr<Gio::SimpleAction> _zoomAction;
    Glib::RefPtr<Gio::SimpleAction> _aspectRatioAction;
};
}
}
}
