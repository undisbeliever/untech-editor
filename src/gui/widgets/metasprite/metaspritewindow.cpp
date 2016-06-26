#include "metaspritewindow.h"
#include "addtilesdialog.h"
#include "gui/widgets/common/controllerinterface.h"
#include "gui/widgets/common/errormessagedialog.h"

using namespace UnTech::Widgets::MetaSprite;

typedef UnTech::Controller::Undo::UndoStack UndoStack;
typedef MS::MetaSpriteController::SelectedTypeController SelectedTypeController;

MetaSpriteWindow::MetaSpriteWindow()
    : Gtk::ApplicationWindow()
    , _controller(std::make_unique<UnTech::Widgets::ControllerInterface>(*this))
    , _editor(_controller)
{
    add(_editor.widget);
    show_all_children();

    // Register actions with window
    add_action("save-as", sigc::mem_fun(*this, &MetaSpriteWindow::do_saveAs));
    add_action("exit", sigc::mem_fun(*this, &MetaSpriteWindow::close));
    add_action("add-tiles", sigc::mem_fun(*this, &MetaSpriteWindow::do_addTiles));

    _saveAction = Gio::SimpleAction::create("save");
    _saveAction->signal_activate().connect(sigc::hide(sigc::mem_fun(
        *this, &MetaSpriteWindow::do_save)));
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
        "set-zoom",
        sigc::mem_fun(_controller.settings(), &Controller::Settings::setZoom),
        _controller.settings().zoom());

    _aspectRatioAction = add_action_radio_integer(
        "set-aspect-ratio",
        [this](int state) {
            _controller.settings().setAspectRatio(
                static_cast<Controller::Settings::AspectRatio>(state));
        },
        (int)_controller.settings().aspectRatio());

    _splitViewAction = add_action_bool(
        "split-view", sigc::mem_fun(*this, &MetaSpriteWindow::do_splitView), false);

    updateItemActions();
    updateUndoActions();

    /*
     * SLOTS
     * =======
     */

    // Controller Signals
    _controller.selectedTypeController().signal_typeChanged().connect(sigc::mem_fun(
        *this, &MetaSpriteWindow::updateItemActions));

    _controller.selectedTypeController().signal_listChanged().connect(sigc::mem_fun(
        *this, &MetaSpriteWindow::updateItemActions));

    _controller.undoStack().signal_stackChanged.connect(sigc::mem_fun(
        *this, &MetaSpriteWindow::updateUndoActions));

    _controller.undoStack().signal_dirtyBitChanged.connect(sigc::mem_fun(
        *this, &MetaSpriteWindow::updateTitle));

    _controller.abstractFrameSetController().signal_nameChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &MetaSpriteWindow::updateTitle)));

    _controller.settings().signal_zoomChanged().connect([this](void) {
        _zoomAction->change_state(_controller.settings().zoom());
        _aspectRatioAction->change_state((int)_controller.settings().aspectRatio());
    });
}

void MetaSpriteWindow::updateTitle()
{
    const auto& undoStack = _controller.undoStack();
    const auto* frameSet = _controller.frameSetController().selected();

    if (frameSet) {
        if (undoStack.isDirty()) {
            set_title("*" + frameSet->name() + _(": Untech MetaSprite Editor"));
            _saveAction->set_enabled(true);
        }
        else {
            set_title(frameSet->name() + _(": Untech MetaSprite Editor"));
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

void MetaSpriteWindow::updateItemActions()
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

void MetaSpriteWindow::do_save()
{
    if (_controller.document()) {
        bool s = _controller.saveDocument();
        if (!s) {
            return do_saveAs();
        }

        updateTitle();
    }
}

void MetaSpriteWindow::do_saveAs()
{
    const auto* document = _controller.document();

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
            dialog.set_current_name(document->frameSet().name() + ".utms");
        }

        int result = dialog.run();

        if (result == Gtk::RESPONSE_OK) {
            // add extension if missing
            auto name = dialog.get_current_name();
            if (dialog.get_filter() == filterUtms && name.find('.') == Glib::ustring::npos) {
                dialog.set_current_name(name + ".utms");
            }

            _controller.saveDocumentAs(dialog.get_filename());

            updateTitle();
        }
    }
}

void MetaSpriteWindow::do_addTiles()
{
    auto& frameSetController = _controller.frameSetController();

    if (frameSetController.selected() != nullptr) {
        AddTilesDialog dialog(frameSetController, *this);
        dialog.run();
    }
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
