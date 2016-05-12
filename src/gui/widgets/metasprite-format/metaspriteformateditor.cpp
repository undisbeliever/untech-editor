#include "metaspriteformateditor.h"
#include "gui/widgets/defaults.h"
#include "gui/widgets/common/errormessagedialog.h"
#include <glibmm/i18n.h>

using namespace UnTech::Widgets::MetaSpriteFormat;
namespace FSEO = UnTech::MetaSpriteFormat::FrameSetExportOrder;

MetaSpriteFormatEditor::MetaSpriteFormatEditor()
    : Gtk::Paned(Gtk::ORIENTATION_VERTICAL)
    , _fseoDocument(nullptr)
    , _fseoBox(Gtk::ORIENTATION_VERTICAL)
    , _fseoGrid()
    , _fseoFilenameBox(Gtk::ORIENTATION_HORIZONTAL)
    , _fseoFilenameEntry()
    , _fseoFilenameButton("...")
    , _fseoNameEntry()
    , _fseoContainer()
    , _fseoTreeView()
    , _fseoFilenameLabel(_("Export Order File:"), Gtk::ALIGN_START)
    , _fseoNameLabel(_("FrameSet Type:"), Gtk::ALIGN_START)
{
    _fseoFilenameEntry.set_sensitive(false);
    _fseoNameEntry.set_sensitive(false);

    _fseoGrid.set_border_width(DEFAULT_BORDER);
    _fseoGrid.set_row_spacing(DEFAULT_ROW_SPACING);

    _fseoFilenameBox.pack_start(_fseoFilenameEntry, true, true);
    _fseoFilenameBox.pack_start(_fseoFilenameButton, false, false);

    _fseoGrid.attach(_fseoFilenameLabel, 0, 0, 1, 1);
    _fseoGrid.attach(_fseoFilenameBox, 1, 0, 3, 1);

    _fseoGrid.attach(_fseoNameLabel, 0, 1, 1, 1);
    _fseoGrid.attach(_fseoNameEntry, 1, 1, 3, 1);

    _fseoContainer.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _fseoContainer.add(_fseoTreeView);

    _fseoBox.pack_start(_fseoGrid, false, false);
    _fseoBox.pack_start(_fseoContainer, true, true);

    this->pack1(_fseoBox, true, false);

    // ::TODO animation editor::

    /*
     * SLOTS
     * =====
     */

    _fseoFilenameButton.signal_clicked().connect(sigc::mem_fun(
        *this, &MetaSpriteFormatEditor::on_fseFilenameButtonClicked));
}

const std::string& MetaSpriteFormatEditor::exportOrderFilename() const
{
    const static std::string emptyString;

    if (_fseoDocument) {
        return _fseoDocument->filename();
    }
    else {
        return emptyString;
    }
}

void MetaSpriteFormatEditor::setFrameSetExportOrderDocument(const std::shared_ptr<const FSEO::ExportOrderDocument>& document)
{
    if (_fseoDocument != document) {
        _fseoDocument = document;

        updateGuiValues();
    }
}

void MetaSpriteFormatEditor::loadFrameSetExportOrderFile(const Glib::ustring& filename)
{
    try {
        _fseoDocument = FSEO::ExportOrderDocument::loadReadOnly(filename);
    }
    catch (const std::exception& ex) {
        _fseoDocument = nullptr;

        showErrorMessage(*this, "Unable to open file", ex);
    }

    updateGuiValues();

    signal_frameSetExportOrderLoaded.emit();
}

void MetaSpriteFormatEditor::updateGuiValues()
{
    if (_fseoDocument != nullptr) {
        _fseoFilenameEntry.set_text(_fseoDocument->filename());
        _fseoFilenameEntry.set_position(-1); // right align text

        _fseoNameEntry.set_text(_fseoDocument->exportOrder().name());
        _fseoTreeView.loadData(_fseoDocument);
    }
    else {
        _fseoFilenameEntry.set_text("");
        _fseoNameEntry.set_text("");
        _fseoTreeView.clear();
    }
}

void MetaSpriteFormatEditor::on_fseFilenameButtonClicked()
{
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

    if (_fseoDocument) {
        dialog.set_filename(_fseoDocument->filename());
    }

    // Set transient parent from widget
    // Could not derefernce RefPtr<Gtk::Window> for base-class constructor
    // using C method instead
    GtkWidget* toplevel = gtk_widget_get_toplevel(GTK_WIDGET(this->gobj()));
    gtk_window_set_transient_for(GTK_WINDOW(dialog.gobj()), GTK_WINDOW(toplevel));

    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK) {
        loadFrameSetExportOrderFile(dialog.get_filename());
    }
}
