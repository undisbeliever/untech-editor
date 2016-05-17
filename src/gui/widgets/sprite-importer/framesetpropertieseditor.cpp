#include "framesetpropertieseditor.h"
#include "signals.h"
#include "gui/undo/actionhelper.h"
#include "gui/undo/mergeactionhelper.h"
#include "gui/widgets/defaults.h"
#include "models/common/string.h"

#include <glibmm/i18n.h>
#include <iomanip>

using namespace UnTech::Widgets::SpriteImporter;

PARAMETER_UNDO_MERGE_ACTION2(frameSet_Grid_merge_setFrameSize,
                             SI::FrameSet, grid, UnTech::usize, frameSize, setFrameSize,
                             Signals::frameSetChanged, Signals::frameSetGridChanged,
                             "Change Grid Frame Size")

PARAMETER_UNDO_MERGE_ACTION2(frameSet_Grid_merge_setOffset,
                             SI::FrameSet, grid, UnTech::upoint, offset, setOffset,
                             Signals::frameSetChanged, Signals::frameSetGridChanged,
                             "Change Grid Offset")

PARAMETER_UNDO_MERGE_ACTION2(frameSet_Grid_merge_setPadding,
                             SI::FrameSet, grid, UnTech::usize, padding, setPadding,
                             Signals::frameSetChanged, Signals::frameSetGridChanged,
                             "Change Grid Padding")

PARAMETER_UNDO_MERGE_ACTION2(frameSet_Grid_merge_setOrigin,
                             SI::FrameSet, grid, UnTech::upoint, origin, setOrigin,
                             Signals::frameSetChanged, Signals::frameSetGridChanged,
                             "Change Grid Origin")

SIMPLE_UNDO_ACTION2(frameSet_setImageFilename,
                    SI::FrameSet, std::string, imageFilename, setImageFilename,
                    Signals::frameSetChanged, Signals::frameSetImageChanged,
                    "Change Image")

FrameSetPropertiesEditor::FrameSetPropertiesEditor(Selection& selection)
    : widget(Gtk::ORIENTATION_VERTICAL)
    , _selection(selection)
    , _abstractEditor(selection)
    , _wgrid()
    , _imageFilenameBox(Gtk::ORIENTATION_HORIZONTAL)
    , _imageFilenameEntry()
    , _imageFilenameButton(_("..."))
    , _transparentColorBox(Gtk::ORIENTATION_HORIZONTAL)
    , _transparentColorEntry()
    , _transparentColorButton()
    , _gridFrameSizeSpinButtons()
    , _gridOffsetSpinButtons()
    , _gridPaddingSpinButtons()
    , _gridOriginSpinButtons()
    , _imageFilenameLabel(_("Image:"), Gtk::ALIGN_START)
    , _transparentColorLabel(_("Transparent:"), Gtk::ALIGN_START)
    , _gridFrameSizeLabel(_("Grid Size:"), Gtk::ALIGN_START)
    , _gridFrameSizeCrossLabel(_(" x "))
    , _gridOffsetLabel(_("Grid Offset:"), Gtk::ALIGN_START)
    , _gridOffsetCommaLabel(_(" , "))
    , _gridPaddingLabel(_("Grid Padding:"), Gtk::ALIGN_START)
    , _gridPaddingCrossLabel(_(" x "))
    , _gridOriginLabel(_("Grid Origin:"), Gtk::ALIGN_START)
    , _gridOriginCommaLabel(_(" , "))
    , _updatingValues(false)
{
    _wgrid.set_border_width(DEFAULT_BORDER);
    _wgrid.set_row_spacing(DEFAULT_ROW_SPACING);

    _imageFilenameEntry.set_sensitive(false);

    _imageFilenameBox.add(_imageFilenameEntry);
    _imageFilenameBox.add(_imageFilenameButton);

    _wgrid.attach(_imageFilenameLabel, 0, 2, 1, 1);
    _wgrid.attach(_imageFilenameBox, 1, 2, 3, 1);

    _transparentColorEntry.set_sensitive(false);
    _transparentColorBox.add(_transparentColorEntry);
    _transparentColorBox.add(_transparentColorButton);

    _wgrid.attach(_transparentColorLabel, 0, 3, 1, 1);
    _wgrid.attach(_transparentColorBox, 1, 3, 3, 1);

    _wgrid.attach(_gridFrameSizeLabel, 0, 4, 1, 1);
    _wgrid.attach(_gridFrameSizeSpinButtons.widthSpin, 1, 4, 1, 1);
    _wgrid.attach(_gridFrameSizeCrossLabel, 2, 4, 1, 1);
    _wgrid.attach(_gridFrameSizeSpinButtons.heightSpin, 3, 4, 1, 1);

    _wgrid.attach(_gridOffsetLabel, 0, 5, 1, 1);
    _wgrid.attach(_gridOffsetSpinButtons.xSpin, 1, 5, 1, 1);
    _wgrid.attach(_gridOffsetCommaLabel, 2, 5, 1, 1);
    _wgrid.attach(_gridOffsetSpinButtons.ySpin, 3, 5, 1, 1);

    _wgrid.attach(_gridPaddingLabel, 0, 6, 1, 1);
    _wgrid.attach(_gridPaddingSpinButtons.widthSpin, 1, 6, 1, 1);
    _wgrid.attach(_gridPaddingCrossLabel, 2, 6, 1, 1);
    _wgrid.attach(_gridPaddingSpinButtons.heightSpin, 3, 6, 1, 1);

    _wgrid.attach(_gridOriginLabel, 0, 7, 1, 1);
    _wgrid.attach(_gridOriginSpinButtons.xSpin, 1, 7, 1, 1);
    _wgrid.attach(_gridOriginCommaLabel, 2, 7, 1, 1);
    _wgrid.attach(_gridOriginSpinButtons.ySpin, 3, 7, 1, 1);

    widget.pack_start(_abstractEditor.widget, true, true);
    widget.pack_start(_wgrid, true, true);

    updateGuiValues();

    /**
     * SLOTS
     */

    /* Update gui when selected frameSet changed */
    _selection.signal_frameSetChanged.connect(sigc::mem_fun(
        *this, &FrameSetPropertiesEditor::updateGuiValues));

    /* Update gui if frameSet has changed */
    Signals::frameSetChanged.connect([this](const SI::FrameSet* frameSet) {
        if (frameSet == _selection.frameSet()) {
            updateGuiValues();
        }
    });

    /* Update transparent color if image changed */
    Signals::frameSetImageChanged.connect([this](const SI::FrameSet* frameSet) {
        if (frameSet && frameSet == _selection.frameSet()) {
            _selection.frameSet()->image();
            updateGuiValues();
        }
    });

    /** Update transparent button when changed */
    _selection.signal_selectTransparentModeChanged.connect([this](void) {
        _transparentColorButton.set_active(_selection.selectTransparentMode());
    });

    /** Transparent button pressed signal */
    _transparentColorButton.signal_toggled().connect([this](void) {
        _selection.setSelectTransparentMode(_transparentColorButton.get_active());
    });

    /** Set Image filename signal */
    _imageFilenameButton.signal_clicked().connect(
        sigc::mem_fun(*this, &FrameSetPropertiesEditor::on_imageFilenameButtonClicked));

    /** Set Grid Frame Size signal */
    _gridFrameSizeSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            frameSet_Grid_merge_setFrameSize(_selection.frameSet(),
                                             _gridFrameSizeSpinButtons.value());
        }
    });
    _gridFrameSizeSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _selection, &Selection::dontMergeNextUndoAction)));

    /** Set Grid Offset signal */
    _gridOffsetSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            frameSet_Grid_merge_setOffset(_selection.frameSet(),
                                          _gridOffsetSpinButtons.value());
        }
    });
    _gridOffsetSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _selection, &Selection::dontMergeNextUndoAction)));

    /** Set Grid Padding signal */
    _gridPaddingSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            frameSet_Grid_merge_setPadding(_selection.frameSet(),
                                           _gridPaddingSpinButtons.value());
        }
    });
    _gridPaddingSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _selection, &Selection::dontMergeNextUndoAction)));

    /** Set Grid Offset signal */
    _gridOriginSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            frameSet_Grid_merge_setOrigin(_selection.frameSet(),
                                          _gridOriginSpinButtons.value());
        }
    });
    _gridOriginSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _selection, &Selection::dontMergeNextUndoAction)));
}

void FrameSetPropertiesEditor::updateGuiValues()
{
    const SI::FrameSet* frameSet = _selection.frameSet();

    if (frameSet) {
        _updatingValues = true;

        _imageFilenameEntry.set_text(frameSet->imageFilename());
        _imageFilenameEntry.set_position(-1); // right align text

        if (frameSet->transparentColorValid()) {
            auto transparent = frameSet->transparentColor();

            auto c = Glib::ustring::format(std::setfill(L'0'), std::setw(6), std::hex,
                                           transparent.rgb());
            _transparentColorEntry.set_text(c);

            _transparentColorButton.set_color(transparent);
        }
        else {
            _transparentColorEntry.set_text("");
            _transparentColorButton.unset_color();
        }

        _gridFrameSizeSpinButtons.set_value(frameSet->grid().frameSize());
        _gridOffsetSpinButtons.set_value(frameSet->grid().offset());
        _gridPaddingSpinButtons.set_value(frameSet->grid().padding());
        _gridOriginSpinButtons.set_value(frameSet->grid().origin());

        _updatingValues = false;

        widget.set_sensitive(true);
    }
    else {
        static const upoint zeroPoint(0, 0);
        static const usize zeroSize(0, 0);

        _imageFilenameEntry.set_text("");
        _transparentColorEntry.set_text("");
        _transparentColorButton.unset_color();

        _gridFrameSizeSpinButtons.set_value(zeroSize);
        _gridOffsetSpinButtons.set_value(zeroPoint);
        _gridPaddingSpinButtons.set_value(zeroSize);
        _gridOriginSpinButtons.set_value(zeroPoint);

        widget.set_sensitive(false);
    }
}

void FrameSetPropertiesEditor::on_imageFilenameButtonClicked()
{
    SI::FrameSet* frameSet = _selection.frameSet();

    if (!frameSet || _updatingValues) {
        return;
    }

    Gtk::FileChooserDialog dialog(_("Select Image"),
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);

    dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("_Open"), Gtk::RESPONSE_OK);
    dialog.set_default_response(Gtk::RESPONSE_OK);

    auto filterPng = Gtk::FileFilter::create();
    filterPng->set_name(_("PNG Imagess"));
    filterPng->add_mime_type("image/png");
    dialog.add_filter(filterPng);

    auto filterAny = Gtk::FileFilter::create();
    filterAny->set_name(_("All files"));
    filterAny->add_pattern("*");
    dialog.add_filter(filterAny);

    if (!frameSet->imageFilename().empty()) {
        dialog.set_filename(frameSet->imageFilename());
    }

    // Set transient parent from widget
    // Could not derefernce RefPtr<Gtk::Window> for base-class constructor
    // using C method instead
    GtkWidget* toplevel = gtk_widget_get_toplevel(GTK_WIDGET(widget.gobj()));
    gtk_window_set_transient_for(GTK_WINDOW(dialog.gobj()), GTK_WINDOW(toplevel));

    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK) {
        frameSet_setImageFilename(frameSet, dialog.get_filename());
    }
}
