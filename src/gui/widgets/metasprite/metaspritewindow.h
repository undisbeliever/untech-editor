#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_METASPRITEWINDOW_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_METASPRITEWINDOW_H_

#include "metaspriteeditor.h"
#include "selection.h"
#include "document.h"
#include "models/metasprite.h"
#include "gui/widgets/defaults.h"

#include <memory>
#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

class MetaSpriteWindow : public Gtk::ApplicationWindow {
public:
    MetaSpriteWindow();

    Document* document() const { return _editor.document(); }

    void setDocument(std::unique_ptr<Document> document);

protected:
    void updateTitle();
    void updateUndoActions();
    void updateItemActions();

    void do_undo();
    void do_redo();
    void do_save();
    void do_saveAs();

    bool on_delete_event(GdkEventAny* any_event);

private:
    MetaSpriteEditor _editor;

    Glib::RefPtr<Gio::SimpleAction> _saveAction;
    Glib::RefPtr<Gio::SimpleAction> _undoAction;
    Glib::RefPtr<Gio::SimpleAction> _redoAction;

    Glib::RefPtr<Gio::SimpleAction> _createSelectedAction;
    Glib::RefPtr<Gio::SimpleAction> _cloneSelectedAction;
    Glib::RefPtr<Gio::SimpleAction> _removeSelectedAction;
    Glib::RefPtr<Gio::SimpleAction> _moveSelectedUpAction;
    Glib::RefPtr<Gio::SimpleAction> _moveSelectedDownAction;

    sigc::connection _undoStackConnection;
    sigc::connection _updateTitleConnection;
};
}
}
}

#endif
