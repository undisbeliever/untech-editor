#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_METASPRITEWINDOW_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_METASPRITEWINDOW_H_

#include "metaspriteeditor.h"
#include "selection.h"
#include "gui/undo/undostack.h"
#include "gui/widgets/defaults.h"
#include "models/metasprite/document.h"

#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <memory>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class MetaSpriteWindow : public Gtk::ApplicationWindow {
public:
    MetaSpriteWindow();

    MS::MetaSpriteDocument* document() const { return _editor.document(); }

    void setDocument(std::unique_ptr<MS::MetaSpriteDocument> document);

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
    void do_splitView();

    bool on_delete_event(GdkEventAny* any_event);

private:
    MetaSpriteEditor _editor;
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
    Glib::RefPtr<Gio::SimpleAction> _splitViewAction;
};
}
}
}

#endif
