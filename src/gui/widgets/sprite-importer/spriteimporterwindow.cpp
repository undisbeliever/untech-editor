#include "spriteimporterwindow.h"

using namespace UnTech::Widgets::SpriteImporter;
namespace SI = UnTech::SpriteImporter;

SpriteImporterWindow::SpriteImporterWindow()
    : Gtk::ApplicationWindow()
    , _editor()
    , _undoStackConnection()
{
    add(_editor.widget);
    show_all_children();

    // Register actions with window
    add_action("save", sigc::mem_fun(*this, &SpriteImporterWindow::do_save));
    add_action("save-as", sigc::mem_fun(*this, &SpriteImporterWindow::do_saveAs));
    add_action("exit", sigc::mem_fun(*this, &SpriteImporterWindow::close));

    _undoAction = Gio::SimpleAction::create("undo");
    _undoAction->signal_activate().connect(
        sigc::hide(sigc::mem_fun(*this, &SpriteImporterWindow::do_undo)));
    add_action(_undoAction);

    _redoAction = Gio::SimpleAction::create("redo");
    _redoAction->signal_activate().connect(
        sigc::hide(sigc::mem_fun(*this, &SpriteImporterWindow::do_redo)));
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

    updateItemActions();
    updateUndoActions();

    /*
     * SIGNALS
     * =======
     */

    _editor.selection().signal_selectionChanged.connect(
        sigc::mem_fun(*this, &SpriteImporterWindow::updateItemActions));

    Signals::frameObjectListChanged.connect(sigc::hide(
        sigc::mem_fun(*this, &SpriteImporterWindow::updateItemActions)));

    Signals::actionPointListChanged.connect(sigc::hide(
        sigc::mem_fun(*this, &SpriteImporterWindow::updateItemActions)));

    Signals::entityHitboxListChanged.connect(sigc::hide(
        sigc::mem_fun(*this, &SpriteImporterWindow::updateItemActions)));
}

void SpriteImporterWindow::setDocument(std::unique_ptr<Document> document)
{
    _undoStackConnection.disconnect();

    if (document) {
        _undoStackConnection = document->undoStack().signal_stackChanged.connect(
            sigc::mem_fun(*this, &SpriteImporterWindow::updateUndoActions));
    }

    _editor.setDocument(std::move(document));

    updateItemActions();
    updateUndoActions();
}

void SpriteImporterWindow::updateUndoActions()
{
    auto* document = _editor.document();

    if (document) {
        const auto& undoStack = document->undoStack();

        _undoAction->set_enabled(undoStack.canUndo());
        _redoAction->set_enabled(undoStack.canRedo());
    }
    else {
        _undoAction->set_enabled(false);
        _redoAction->set_enabled(false);
    }
}

void SpriteImporterWindow::updateItemActions()
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

void SpriteImporterWindow::do_undo()
{
    auto* document = _editor.document();

    if (document) {
        document->undoStack().undo();
    }
}

void SpriteImporterWindow::do_redo()
{
    auto* document = _editor.document();

    if (document) {
        document->undoStack().redo();
    }
}

void SpriteImporterWindow::do_save()
{
    auto* document = _editor.document();

    if (document) {
        if (document->filename().empty()) {
            return do_saveAs();
        }
        else {
            try {
                document->save();
            }
            catch (const std::exception& ex) {
                // ::TODO replace with error dialog::
                std::cerr << "Unable to save file: " << ex.what() << std::endl;
            }
        }
    }
}

void SpriteImporterWindow::do_saveAs()
{
    auto* document = _editor.document();

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
            if (document->frameSet()) {
                dialog.set_current_name(document->frameSet()->name() + ".utsi");
            }
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
            }
            catch (const std::exception& ex) {
                // ::TODO replace with error dialog::
                std::cerr << "Unable to save file: " << ex.what() << std::endl;
            }
        }
    }
}
