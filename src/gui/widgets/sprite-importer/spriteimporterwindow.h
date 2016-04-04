#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SPRITEIMPORTERACTIONS_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SPRITEIMPORTERACTIONS_H_

#include "spriteimportereditor.h"
#include "selection.h"
#include "document.h"
#include "models/sprite-importer.h"
#include "gui/widgets/defaults.h"

#include <memory>
#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

class SpriteImporterWindow : public Gtk::ApplicationWindow {
public:
    SpriteImporterWindow();

    Document* document() const { return _editor.document(); }

    void setDocument(std::unique_ptr<Document> document);

protected:
    void updateUndoActions();
    void updateItemActions();

    void do_undo();
    void do_redo();

private:
    SpriteImporterEditor _editor;

    Glib::RefPtr<Gio::SimpleAction> _undoAction;
    Glib::RefPtr<Gio::SimpleAction> _redoAction;

    Glib::RefPtr<Gio::SimpleAction> _createSelectedAction;
    Glib::RefPtr<Gio::SimpleAction> _cloneSelectedAction;
    Glib::RefPtr<Gio::SimpleAction> _removeSelectedAction;
    Glib::RefPtr<Gio::SimpleAction> _moveSelectedUpAction;
    Glib::RefPtr<Gio::SimpleAction> _moveSelectedDownAction;

    sigc::connection _undoStackConnection;
};
}
}
}

#endif
