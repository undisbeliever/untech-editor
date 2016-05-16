#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SPRITEIMPORTERWINDOW_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SPRITEIMPORTERWINDOW_H_

#include "spriteimportereditor.h"
#include "selection.h"
#include "models/sprite-importer.h"
#include "gui/undo/undostack.h"
#include "gui/widgets/defaults.h"

#include <memory>
#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class SpriteImporterWindow : public Gtk::ApplicationWindow {
public:
    SpriteImporterWindow();

    SI::SpriteImporterDocument* document() const { return _editor.document(); }

    void setDocument(std::unique_ptr<SI::SpriteImporterDocument> document);

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
    SpriteImporterEditor _editor;
    Undo::UndoStack _undoStack;

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

#endif
