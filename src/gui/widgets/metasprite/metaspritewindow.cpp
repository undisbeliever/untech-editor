#include "metaspritewindow.h"
#include "../metasprite-format/signals.h"
#include "../common/errormessagedialog.h"

using namespace UnTech::Widgets::MetaSprite;
namespace MS = UnTech::MetaSprite;
namespace MSF = UnTech::MetaSpriteFormat;
namespace WMSF = UnTech::Widgets::MetaSpriteFormat;

MetaSpriteWindow::MetaSpriteWindow()
    : Gtk::ApplicationWindow()
    , _editor()
    , _undoStack()
{
    add(_editor.widget);
    show_all_children();

    // Register actions with window
    add_action("save-as", sigc::mem_fun(*this, &MetaSpriteWindow::do_saveAs));
    add_action("exit", sigc::mem_fun(*this, &MetaSpriteWindow::close));

    _saveAction = Gio::SimpleAction::create("save");
    _saveAction->signal_activate().connect(
        sigc::hide(sigc::mem_fun(*this, &MetaSpriteWindow::do_save)));
    add_action(_saveAction);

    _undoAction = Gio::SimpleAction::create("undo");
    _undoAction->signal_activate().connect(
        sigc::hide(sigc::mem_fun(*this, &MetaSpriteWindow::do_undo)));
    add_action(_undoAction);

    _redoAction = Gio::SimpleAction::create("redo");
    _redoAction->signal_activate().connect(
        sigc::hide(sigc::mem_fun(*this, &MetaSpriteWindow::do_redo)));
    add_action(_redoAction);

    _createSelectedAction = Gio::SimpleAction::create("create-selected");
    _createSelectedAction->signal_activate().connect(
        sigc::hide(sigc::mem_fun(_editor.selection(), &Selection::createNewOfSelectedType)));
    add_action(_createSelectedAction);

    _cloneSelectedAction = Gio::SimpleAction::create("clone-selected");
    _cloneSelectedAction->signal_activate().connect(
        sigc::hide(sigc::mem_fun(_editor.selection(), &Selection::cloneSelected)));
    add_action(_cloneSelectedAction);

    _removeSelectedAction = Gio::SimpleAction::create("remove-selected");
    _removeSelectedAction->signal_activate().connect(
        sigc::hide(sigc::mem_fun(_editor.selection(), &Selection::removeSelected)));
    add_action(_removeSelectedAction);

    _moveSelectedUpAction = Gio::SimpleAction::create("move-selected-up");
    _moveSelectedUpAction->signal_activate().connect(
        sigc::hide(sigc::mem_fun(_editor.selection(), &Selection::moveSelectedUp)));
    add_action(_moveSelectedUpAction);

    _moveSelectedDownAction = Gio::SimpleAction::create("move-selected-down");
    _moveSelectedDownAction->signal_activate().connect(
        sigc::hide(sigc::mem_fun(_editor.selection(), &Selection::moveSelectedDown)));
    add_action(_moveSelectedDownAction);

    _zoomAction = add_action_radio_integer(
        "set-zoom", sigc::mem_fun(*this, &MetaSpriteWindow::do_setZoom), DEFAULT_ZOOM);

    _aspectRatioAction = add_action_radio_integer(
        "set-aspect-ratio", sigc::mem_fun(*this, &MetaSpriteWindow::do_setAspectRatio), 1);

    _splitViewAction = add_action_bool(
        "split-view", sigc::mem_fun(*this, &MetaSpriteWindow::do_splitView), false);

    updateItemActions();
    updateUndoActions();
    updateGuiZoom();

    /*
     * SIGNALS
     * =======
     */

    _editor.selection().signal_selectionChanged.connect(
        sigc::mem_fun(*this, &MetaSpriteWindow::updateItemActions));

    _undoStack.signal_stackChanged.connect(
        sigc::mem_fun(*this, &MetaSpriteWindow::updateUndoActions));

    _undoStack.signal_dirtyBitChanged.connect(
        sigc::mem_fun(*this, &MetaSpriteWindow::updateTitle));

    Signals::frameObjectListChanged.connect(sigc::hide(
        sigc::mem_fun(*this, &MetaSpriteWindow::updateItemActions)));

    Signals::actionPointListChanged.connect(sigc::hide(
        sigc::mem_fun(*this, &MetaSpriteWindow::updateItemActions)));

    Signals::entityHitboxListChanged.connect(sigc::hide(
        sigc::mem_fun(*this, &MetaSpriteWindow::updateItemActions)));

    // Update title when frameset name changes
    WMSF::Signals::abstractFrameSetNameChanged.connect([this](const MSF::AbstractFrameSet* frameSet) {
        auto* document = _editor.document();
        if (document && frameSet == &document->frameSet()) {
            updateTitle();
        }
    });
}

void MetaSpriteWindow::setDocument(std::unique_ptr<MS::MetaSpriteDocument> document)
{
    auto* oldDocument = _editor.document();
    if (oldDocument) {
        oldDocument->setUndoStack(nullptr);
    }

    _undoStack.clear();

    if (document) {
        document->setUndoStack(&_undoStack);
    }

    _editor.setDocument(std::move(document));

    updateItemActions();
    updateTitle();
    updateUndoActions();
}

void MetaSpriteWindow::updateTitle()
{
    auto* document = _editor.document();

    if (document) {
        if (_undoStack.isDirty()) {
            set_title("*" + document->frameSet().name() + _(": Untech MetaSprite Editor"));
            _saveAction->set_enabled(true);
        }
        else {
            set_title(document->frameSet().name() + _(": Untech MetaSprite Editor"));
            _saveAction->set_enabled(false);
        }
    }
    else {
        set_title(_(": Untech Sprite Importer"));
        _saveAction->set_enabled(false);
    }
}

void MetaSpriteWindow::updateUndoActions()
{
    auto* document = _editor.document();

    if (document) {
        _undoAction->set_enabled(_undoStack.canUndo());
        _redoAction->set_enabled(_undoStack.canRedo());
        _saveAction->set_enabled(_undoStack.isDirty());
    }
    else {
        _undoAction->set_enabled(false);
        _redoAction->set_enabled(false);
        _saveAction->set_enabled(false);
    }
}

void MetaSpriteWindow::updateItemActions()
{
    auto selection = _editor.selection();

    bool canCrud = selection.canCrudSelected();
    bool canMoveUp = selection.canMoveSelectedUp();
    bool canMoveDown = selection.canMoveSelectedDown();

    _createSelectedAction->set_enabled(canCrud);
    _cloneSelectedAction->set_enabled(canCrud);
    _removeSelectedAction->set_enabled(canCrud);
    _moveSelectedUpAction->set_enabled(canMoveUp);
    _moveSelectedDownAction->set_enabled(canMoveDown);
}

void MetaSpriteWindow::do_undo()
{
    _undoStack.undo();
}

void MetaSpriteWindow::do_redo()
{
    _undoStack.redo();
}

void MetaSpriteWindow::do_save()
{
    auto* document = _editor.document();

    if (document) {
        if (document->filename().empty()) {
            return do_saveAs();
        }
        else {
            try {
                document->save();
                _undoStack.markClean();
            }
            catch (const std::exception& ex) {
                showErrorMessage(this, "Unable to save file", ex);
            }
        }

        updateTitle();
    }
}

void MetaSpriteWindow::do_saveAs()
{
    auto* document = _editor.document();

    if (document) {
        Gtk::FileChooserDialog dialog(*this,
                                      _("Save As"),
                                      Gtk::FILE_CHOOSER_ACTION_SAVE);

        dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
        dialog.add_button(_("_Open"), Gtk::RESPONSE_OK);
        dialog.set_default_response(Gtk::RESPONSE_OK);

        auto filterUtms = Gtk::FileFilter::create();
        filterUtms->set_name(_("UnTech MetaSprite File"));
        filterUtms->add_pattern("*.utms");
        dialog.add_filter(filterUtms);

        auto filterAny = Gtk::FileFilter::create();
        filterAny->set_name(_("All files"));
        filterAny->add_pattern("*");
        dialog.add_filter(filterAny);

        if (!document->filename().empty()) {
            dialog.set_filename(document->filename());
        }
        else {
            dialog.set_current_name(document->frameSet().name() + ".utsi");
        }

        int result = dialog.run();

        if (result == Gtk::RESPONSE_OK) {
            // add extension if missing
            auto name = dialog.get_current_name();
            if (dialog.get_filter() == filterUtms && name.find('.') == Glib::ustring::npos) {
                dialog.set_current_name(name + ".utms");
            }

            try {
                document->saveFile(dialog.get_filename());
                _undoStack.markClean();
            }
            catch (const std::exception& ex) {
                showErrorMessage(this, "Unable to save file", ex);
            }

            updateTitle();
        }
    }
}

void MetaSpriteWindow::do_setZoom(int zoom)
{
    _zoomAction->change_state(zoom);

    updateGuiZoom();
}

void MetaSpriteWindow::do_setAspectRatio(int state)
{
    _aspectRatioAction->change_state(state);

    updateGuiZoom();
}

void MetaSpriteWindow::updateGuiZoom()
{
    int zoom;
    _zoomAction->get_state(zoom);

    double aspect;
    int aState;
    _aspectRatioAction->get_state(aState);
    switch (aState) {
    case 1:
        aspect = NTSC_ASPECT;
        break;

    case 2:
        aspect = PAL_ASPECT;
        break;

    default:
        aspect = 1.0;
    };

    _editor.setZoom(zoom, aspect);
}

void MetaSpriteWindow::do_splitView()
{
    bool state;
    _splitViewAction->get_state(state);

    // have to invert state manually
    state = !state;
    _splitViewAction->change_state(state);

    _editor.setShowTwoEditors(state);
}

bool MetaSpriteWindow::on_delete_event(GdkEventAny*)
{
    auto* document = _editor.document();

    if (document == nullptr) {
        return false;
    }

    if (_undoStack.isDirty()) {
        Gtk::MessageDialog dialog(*this,
                                  _("Do you want to save?"),
                                  false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE);

        dialog.set_secondary_text(_("If you close without saving, your changes will be discarded."));

        dialog.add_button(_("Close without saving"), Gtk::RESPONSE_NO);
        dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
        dialog.add_button(_("_Save"), Gtk::RESPONSE_OK);

        dialog.set_default_response(Gtk::RESPONSE_OK);

        int result = dialog.run();

        if (result == Gtk::RESPONSE_OK) {
            do_save();
        };

        // do not delete if response is cancel
        return result == Gtk::RESPONSE_CANCEL;
    }
    else {
        return false;
    }
}
