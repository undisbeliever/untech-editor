#include "abstractframesetpropertieseditor.h"
#include "signals.h"
#include "gui/undo/actionhelper.h"
#include "gui/widgets/common/errormessagedialog.h"
#include "gui/widgets/defaults.h"
#include <glibmm/i18n.h>

using namespace UnTech::Widgets::MetaSpriteFormat;
namespace FSEO = UnTech::MetaSpriteFormat::FrameSetExportOrder;

SIMPLE_UNDO_ACTION2(frameSet_setName,
                    MSF::AbstractFrameSet, std::string, name, setName,
                    Signals::abstractFrameSetChanged, Signals::abstractFrameSetNameChanged,
                    "Change Name")

SIMPLE_UNDO_ACTION(frameSet_setTilesetType,
                   MSF::AbstractFrameSet, MSF::TilesetType, tilesetType, setTilesetType,
                   Signals::abstractFrameSetChanged,
                   "Change Tileset Type")

/*
 *  Cannot use a simple action because loadExportDocument may raise an exception.
 *  Currently fails silently and displays error in stderr
 *  Not sure if I should get it to display an error message dialog
 *  (and I probably only want to show it once anyways)
 *  ::SHOULDO think about this::
 */
void frameSet_setExportOrderFilename(MSF::AbstractFrameSet* frameSet, const std::string& filename)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(MSF::AbstractFrameSet* frameSet,
               const std::string& oldFilename,
               const std::string& newFilename)
            : _frameSet(frameSet)
            , _oldFilename(oldFilename)
            , _newFilename(newFilename)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            try {
                _frameSet->loadExportOrderDocument(_oldFilename);
            }
            catch (const std::exception& ex) {
                std::cerr << "Error loading export order document: " << _oldFilename
                          << "\n\t" << ex.what() << std::endl;
            }

            Signals::abstractFrameSetChanged.emit(_frameSet);
            Signals::abstractFrameSetExportOrderChanged.emit(_frameSet);
        }

        virtual void redo() override
        {
            try {
                _frameSet->loadExportOrderDocument(_newFilename);
            }
            catch (const std::exception& ex) {
                std::cerr << "Error loading export order document: " << _newFilename
                          << "\n\t" << ex.what() << std::endl;
            }

            Signals::abstractFrameSetChanged.emit(_frameSet);
            Signals::abstractFrameSetExportOrderChanged.emit(_frameSet);
        }

        virtual const Glib::ustring& message() const override
        {
            const static Glib::ustring message = _("Load Export Order Document");
            return message;
        }

    private:
        MSF::AbstractFrameSet* _frameSet;
        const std::string _oldFilename;
        const std::string _newFilename;
    };

    if (frameSet) {
        const std::string& oldFilename = frameSet->exportOrderFilename();

        if (oldFilename != filename) {
            try {
                frameSet->loadExportOrderDocument(filename);
            }
            catch (const std::exception& ex) {
                std::cerr << "Error loading export order document: " << filename
                          << "\n\t" << ex.what() << std::endl;
            }

            const std::string& newFilename = frameSet->exportOrderFilename();
            if (oldFilename != newFilename) {
                Signals::abstractFrameSetChanged.emit(frameSet);
                Signals::abstractFrameSetExportOrderChanged.emit(frameSet);

                UnTech::Undo::UndoStack* undoStack = frameSet->document().undoStack();
                if (undoStack) {
                    undoStack->add_undo(std::make_unique<Action>(
                        frameSet, oldFilename, newFilename));
                }
            }
        }
    }
}

AbstractFrameSetPropertiesEditor::AbstractFrameSetPropertiesEditor(Selection& selection)
    : widget(Gtk::ORIENTATION_VERTICAL)
    , _selection(selection)
    , _grid()
    , _nameEntry()
    , _tilesetTypeCombo()
    , _fseoFilenameBox(Gtk::ORIENTATION_HORIZONTAL)
    , _fseoFilenameEntry()
    , _fseoFilenameButton("...")
    , _fseoNameEntry()
    , _fseoContainer()
    , _fseoTreeView()
    , _nameLabel(_("Name:"), Gtk::ALIGN_START)
    , _tilesetTypeLabel(_("Tileset Type:"), Gtk::ALIGN_START)
    , _fseoFilenameLabel(_("Export Order File:"), Gtk::ALIGN_START)
    , _fseoNameLabel(_("FrameSet Type:"), Gtk::ALIGN_START)
    , _updatingValues(false)
{
    _fseoFilenameEntry.set_sensitive(false);
    _fseoNameEntry.set_sensitive(false);

    _grid.set_border_width(DEFAULT_BORDER);
    _grid.set_row_spacing(DEFAULT_ROW_SPACING);

    _grid.attach(_nameLabel, 0, 0, 1, 1);
    _grid.attach(_nameEntry, 1, 0, 3, 1);

    _grid.attach(_tilesetTypeLabel, 0, 1, 1, 1);
    _grid.attach(_tilesetTypeCombo, 1, 1, 3, 1);

    _fseoFilenameBox.pack_start(_fseoFilenameEntry, true, true);
    _fseoFilenameBox.pack_start(_fseoFilenameButton, false, false);

    _grid.attach(_fseoFilenameLabel, 0, 2, 1, 1);
    _grid.attach(_fseoFilenameBox, 1, 2, 3, 1);

    _grid.attach(_fseoNameLabel, 0, 3, 1, 1);
    _grid.attach(_fseoNameEntry, 1, 3, 3, 1);

    _fseoContainer.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _fseoContainer.add(_fseoTreeView);

    widget.pack_start(_grid, false, false);
    widget.pack_start(_fseoContainer, true, true);

    // ::TODO animation editor::

    updateGuiValues();
    updateGuiTree();

    /*
     * SLOTS
     * =====
     */

    /** FrameSet Updated signal */
    Signals::abstractFrameSetChanged.connect([this](const MSF::AbstractFrameSet* frameSet) {
        if (frameSet == _selection.abstractFrameSet()) {
            updateGuiValues();
        }
    });

    Signals::abstractFrameSetExportOrderChanged.connect([this](const MSF::AbstractFrameSet* frameSet) {
        if (frameSet == _selection.abstractFrameSet()) {
            updateGuiValues();
            updateGuiTree();
        }
    });

    /** Selected FrameSet changed signal */
    _selection.signal_frameSetChanged.connect([this](void) {
        updateGuiValues();
        updateGuiTree();
    });

    /** FrameSet Updated signal */
    // ::TODO move this to _selection::
    Signals::abstractFrameSetChanged.connect([this](const MSF::AbstractFrameSet* frameSet) {
        if (frameSet == _selection.abstractFrameSet()) {
            updateGuiValues();
            updateGuiTree();
        }
    });

    /* Set Parameter has finished editing */
    // signal_editing_done does not work
    // using activate and focus out instead.
    _nameEntry.signal_activate().connect([this](void) {
        if (!_updatingValues) {
            frameSet_setName(_selection.abstractFrameSet(),
                             _nameEntry.get_text());
        }
    });
    _nameEntry.signal_focus_out_event().connect([this](GdkEventFocus*) {
        if (!_updatingValues) {
            frameSet_setName(_selection.abstractFrameSet(),
                             _nameEntry.get_text());
        }
        return false;
    });

    /** Tileset type signal */
    _tilesetTypeCombo.signal_changed().connect([this](void) {
        if (!_updatingValues) {
            frameSet_setTilesetType(_selection.abstractFrameSet(),
                                    _tilesetTypeCombo.get_value());
        }
    });

    _fseoFilenameButton.signal_clicked().connect(sigc::mem_fun(
        *this, &AbstractFrameSetPropertiesEditor::on_fseoFilenameButtonClicked));
}

void AbstractFrameSetPropertiesEditor::updateGuiValues()
{
    const MSF::AbstractFrameSet* frameSet = _selection.abstractFrameSet();

    if (frameSet) {
        _updatingValues = true;

        _nameEntry.set_text(frameSet->name());
        _tilesetTypeCombo.set_value(frameSet->tilesetType());

        _updatingValues = false;

        widget.set_sensitive(true);
    }
    else {

        _nameEntry.set_text("");
        _tilesetTypeCombo.unset_value();

        widget.set_sensitive(false);
    }
}

void AbstractFrameSetPropertiesEditor::updateGuiTree()
{
    const MSF::AbstractFrameSet* frameSet = _selection.abstractFrameSet();

    if (frameSet && frameSet->exportOrderDocument()) {
        const auto& fseoDocument = frameSet->exportOrderDocument();

        _updatingValues = true;

        _fseoFilenameEntry.set_text(fseoDocument->filename());
        _fseoFilenameEntry.set_position(-1); // right align text

        _fseoNameEntry.set_text(fseoDocument->exportOrder().name());
        _fseoTreeView.loadData(fseoDocument);

        _updatingValues = false;
    }
    else {
        _fseoFilenameEntry.set_text("");
        _fseoNameEntry.set_text("");
        _fseoTreeView.clear();
    }
}

void AbstractFrameSetPropertiesEditor::on_fseoFilenameButtonClicked()
{
    MSF::AbstractFrameSet* frameSet = _selection.abstractFrameSet();

    if (frameSet == nullptr) {
        return;
    }

    Gtk::FileChooserDialog dialog(_("Select Export Order File"),
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);

    dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("_Open"), Gtk::RESPONSE_OK);
    dialog.set_default_response(Gtk::RESPONSE_OK);

    auto filterUtsi = Gtk::FileFilter::create();
    filterUtsi->set_name(_("UnTech FrameSet Type File"));
    filterUtsi->add_pattern("*.utft");
    dialog.add_filter(filterUtsi);

    auto filterAny = Gtk::FileFilter::create();
    filterAny->set_name(_("All files"));
    filterAny->add_pattern("*");
    dialog.add_filter(filterAny);

    if (frameSet->exportOrderDocument()) {
        dialog.set_filename(frameSet->exportOrderDocument()->filename());
    }

    // Set transient parent from widget
    // Could not derefernce RefPtr<Gtk::Window> for base-class constructor
    // using C method instead
    GtkWidget* toplevel = gtk_widget_get_toplevel(GTK_WIDGET(widget.gobj()));
    gtk_window_set_transient_for(GTK_WINDOW(dialog.gobj()), GTK_WINDOW(toplevel));

    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK) {
        std::shared_ptr<const MSF::FrameSetExportOrder::ExportOrderDocument> document;
        try {
            document = FSEO::ExportOrderDocument::loadReadOnly(dialog.get_filename());

            frameSet_setExportOrderFilename(frameSet, document->filename());
        }
        catch (const std::exception& ex) {
            showErrorMessage(widget, "Unable to open file", ex);

            frameSet_setExportOrderFilename(frameSet, "");
        }
    }
}
