#include "spriteimporterwindow.h"
#include "gui/widgets/common/errormessagedialog.h"
#include "gui/widgets/defaults.h"

using namespace UnTech::Widgets::SpriteImporter;
namespace MSF = UnTech::MetaSpriteFormat;

typedef UnTech::Undo::UndoStack UndoStack;
typedef SI::SpriteImporterController::SelectedTypeController SelectedTypeController;

SpriteImporterWindow::SpriteImporterWindow()
    : Gtk::ApplicationWindow()
    , _controller(*this)
    , _editor(_controller)
{
    add(_editor.widget);
    show_all_children();

    // Register actions with window
    add_action("save-as", sigc::mem_fun(*this, &SpriteImporterWindow::do_saveAs));
    add_action("exit", sigc::mem_fun(*this, &SpriteImporterWindow::close));

    _saveAction = Gio::SimpleAction::create("save");
    _saveAction->signal_activate().connect(
        sigc::hide(sigc::mem_fun(*this, &SpriteImporterWindow::do_save)));
    add_action(_saveAction);

    _undoAction = Gio::SimpleAction::create("undo");
    _undoAction->signal_activate().connect(sigc::hide(sigc::mem_fun(
        _controller.undoStack(), &UndoStack::undo)));
    add_action(_undoAction);

    _redoAction = Gio::SimpleAction::create("redo");
    _redoAction->signal_activate().connect(sigc::hide(sigc::mem_fun(
        _controller.undoStack(), &UndoStack::redo)));
    add_action(_redoAction);

    _createSelectedAction = Gio::SimpleAction::create("create-selected");
    _createSelectedAction->signal_activate().connect(sigc::hide(sigc::mem_fun(
        _controller.selectedTypeController(), &SelectedTypeController::createNewOfSelectedType)));
    add_action(_createSelectedAction);

    _cloneSelectedAction = Gio::SimpleAction::create("clone-selected");
    _cloneSelectedAction->signal_activate().connect(sigc::hide(sigc::mem_fun(
        _controller.selectedTypeController(), &SelectedTypeController::cloneSelected)));
    add_action(_cloneSelectedAction);

    _removeSelectedAction = Gio::SimpleAction::create("remove-selected");
    _removeSelectedAction->signal_activate().connect(sigc::hide(sigc::mem_fun(
        _controller.selectedTypeController(), &SelectedTypeController::removeSelected)));
    add_action(_removeSelectedAction);

    _moveSelectedUpAction = Gio::SimpleAction::create("move-selected-up");
    _moveSelectedUpAction->signal_activate().connect(sigc::hide(sigc::mem_fun(
        _controller.selectedTypeController(), &SelectedTypeController::moveSelectedUp)));
    add_action(_moveSelectedUpAction);

    _moveSelectedDownAction = Gio::SimpleAction::create("move-selected-down");
    _moveSelectedDownAction->signal_activate().connect(sigc::hide(sigc::mem_fun(
        _controller.selectedTypeController(), &SelectedTypeController::moveSelectedDown)));
    add_action(_moveSelectedDownAction);

    _zoomAction = add_action_radio_integer(
        "set-zoom", sigc::mem_fun(*this, &SpriteImporterWindow::do_setZoom), DEFAULT_ZOOM);

    _aspectRatioAction = add_action_radio_integer(
        "set-aspect-ratio", sigc::mem_fun(*this, &SpriteImporterWindow::do_setAspectRatio), 1);

    updateItemActions();
    updateUndoActions();
    updateGuiZoom();

    /*
     * SIGNALS
     * =======
     */

    // Controller Signals
    _controller.selectedTypeController().signal_typeChanged().connect(sigc::mem_fun(
        *this, &SpriteImporterWindow::updateItemActions));

    _controller.selectedTypeController().signal_listChanged().connect(sigc::mem_fun(
        *this, &SpriteImporterWindow::updateItemActions));

    _controller.undoStack().signal_stackChanged.connect(sigc::mem_fun(
        *this, &SpriteImporterWindow::updateUndoActions));

    _controller.undoStack().signal_dirtyBitChanged.connect(sigc::mem_fun(
        *this, &SpriteImporterWindow::updateTitle));

    _controller.abstractFrameSetController().signal_nameChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &SpriteImporterWindow::updateTitle)));
}

void SpriteImporterWindow::updateTitle()
{
    const auto* document = _controller.document();
    const auto& undoStack = _controller.undoStack();
    const auto* frameSet = _controller.frameSetController().selected();

    if (document) {
        if (undoStack.isDirty()) {
            set_title("*" + frameSet->name() + _(": Untech Sprite Importer"));
            _saveAction->set_enabled(true);
        }
        else {
            set_title(frameSet->name() + _(": Untech Sprite Importer"));
            _saveAction->set_enabled(false);
        }
    }
    else {
        set_title(_(": Untech Sprite Importer"));
        _saveAction->set_enabled(false);
    }
}

void SpriteImporterWindow::updateUndoActions()
{
    const auto* document = _controller.document();
    const auto& undoStack = _controller.undoStack();

    if (document) {
        _undoAction->set_enabled(undoStack.canUndo());
        _redoAction->set_enabled(undoStack.canRedo());
        _saveAction->set_enabled(undoStack.isDirty());
    }
    else {
        _undoAction->set_enabled(false);
        _redoAction->set_enabled(false);
        _saveAction->set_enabled(false);
    }
}

void SpriteImporterWindow::updateItemActions()
{
    auto& selectedType = _controller.selectedTypeController();

    bool canCrud = selectedType.canCrudSelected();
    bool canMoveUp = selectedType.canMoveSelectedUp();
    bool canMoveDown = selectedType.canMoveSelectedDown();

    _createSelectedAction->set_enabled(canCrud);
    _cloneSelectedAction->set_enabled(canCrud);
    _removeSelectedAction->set_enabled(canCrud);
    _moveSelectedUpAction->set_enabled(canMoveUp);
    _moveSelectedDownAction->set_enabled(canMoveDown);
}

void SpriteImporterWindow::do_save()
{
    auto* document = _controller.document();

    if (document) {
        if (document->filename().empty()) {
            return do_saveAs();
        }
        else {
            try {
                document->save();
                // ::TODO move elsewhere::
                _controller.undoStack().markClean();
            }
            catch (const std::exception& ex) {
                showErrorMessage(this, "Unable to save file", ex);
            }
        }

        updateTitle();
    }
}

void SpriteImporterWindow::do_saveAs()
{
    auto* document = _controller.document();

    if (document) {
        Gtk::FileChooserDialog dialog(*this,
                                      _("Save As"),
                                      Gtk::FILE_CHOOSER_ACTION_SAVE);

        dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
        dialog.add_button(_("_Open"), Gtk::RESPONSE_OK);
        dialog.set_default_response(Gtk::RESPONSE_OK);

        auto filterUtsi = Gtk::FileFilter::create();
        filterUtsi->set_name(_("UnTech Sprite Importer File"));
        filterUtsi->add_pattern("*.utsi");
        dialog.add_filter(filterUtsi);

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
            if (dialog.get_filter() == filterUtsi && name.find('.') == Glib::ustring::npos) {
                dialog.set_current_name(name + ".utsi");
            }

            try {
                document->saveFile(dialog.get_filename());
                // ::TODO move elsewhere::
                _controller.undoStack().markClean();
            }
            catch (const std::exception& ex) {
                showErrorMessage(this, "Unable to save file", ex);
            }

            updateTitle();
        }
    }
}

void SpriteImporterWindow::do_setZoom(int zoom)
{
    _zoomAction->change_state(zoom);

    updateGuiZoom();
}

void SpriteImporterWindow::do_setAspectRatio(int state)
{
    _aspectRatioAction->change_state(state);

    updateGuiZoom();
}

void SpriteImporterWindow::updateGuiZoom()
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

bool SpriteImporterWindow::on_delete_event(GdkEventAny*)
{
    auto* document = _controller.document();
    const auto& undoStack = _controller.undoStack();

    if (document == nullptr) {
        return false;
    }

    if (undoStack.isDirty()) {
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
