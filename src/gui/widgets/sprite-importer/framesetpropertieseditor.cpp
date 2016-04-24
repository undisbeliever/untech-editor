#include "framesetpropertieseditor.h"
#include "gui/undo/actionhelper.h"
#include <iomanip>

using namespace UnTech::Widgets::SpriteImporter;

SIMPLE_UNDO_ACTION(frameSet_setName,
                   SI::FrameSet, std::string, name, setName,
                   Signals::frameSetChanged,
                   "Change Name")

PARAMETER_UNDO_ACTION2(frameSet_Grid_setFrameSize,
                       SI::FrameSet, grid, UnTech::usize, frameSize, setFrameSize,
                       Signals::frameSetChanged, Signals::frameSetGridChanged,
                       "Change Grid Frame Size")

PARAMETER_UNDO_ACTION2(frameSet_Grid_setOffset,
                       SI::FrameSet, grid, UnTech::upoint, offset, setOffset,
                       Signals::frameSetChanged, Signals::frameSetGridChanged,
                       "Change Grid Offset")

PARAMETER_UNDO_ACTION2(frameSet_Grid_setPadding,
                       SI::FrameSet, grid, UnTech::usize, padding, setPadding,
                       Signals::frameSetChanged, Signals::frameSetGridChanged,
                       "Change Grid Padding")

PARAMETER_UNDO_ACTION2(frameSet_Grid_setOrigin,
                       SI::FrameSet, grid, UnTech::upoint, origin, setOrigin,
                       Signals::frameSetChanged, Signals::frameSetGridChanged,
                       "Change Grid Origin")

SIMPLE_UNDO_ACTION2(frameSet_setImageFilename,
                    SI::FrameSet, std::string, imageFilename, setImageFilename,
                    Signals::frameSetChanged, Signals::frameSetImageChanged,
                    "Change Image")

FrameSetPropertiesEditor::FrameSetPropertiesEditor(Selection& selection)
    : widget()
    , _frameSet(nullptr)
    , _nameEntry()
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
    , _nameLabel(_("Name:"), Gtk::ALIGN_START)
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
    , _selection(selection)
    , _updatingValues(false)
{
    widget.set_border_width(DEFAULT_BORDER);
    widget.set_row_spacing(DEFAULT_ROW_SPACING);

    widget.attach(_nameLabel, 0, 0, 1, 1);
    widget.attach(_nameEntry, 1, 0, 3, 1);

    _imageFilenameEntry.set_sensitive(false);

    _imageFilenameBox.add(_imageFilenameEntry);
    _imageFilenameBox.add(_imageFilenameButton);

    widget.attach(_imageFilenameLabel, 0, 1, 1, 1);
    widget.attach(_imageFilenameBox, 1, 1, 3, 1);

    _transparentColorEntry.set_sensitive(false);
    _transparentColorBox.add(_transparentColorEntry);
    _transparentColorBox.add(_transparentColorButton);

    widget.attach(_transparentColorLabel, 0, 2, 1, 1);
    widget.attach(_transparentColorBox, 1, 2, 3, 1);

    widget.attach(_gridFrameSizeLabel, 0, 3, 1, 1);
    widget.attach(_gridFrameSizeSpinButtons.widthSpin, 1, 3, 1, 1);
    widget.attach(_gridFrameSizeCrossLabel, 2, 3, 1, 1);
    widget.attach(_gridFrameSizeSpinButtons.heightSpin, 3, 3, 1, 1);

    widget.attach(_gridOffsetLabel, 0, 4, 1, 1);
    widget.attach(_gridOffsetSpinButtons.xSpin, 1, 4, 1, 1);
    widget.attach(_gridOffsetCommaLabel, 2, 4, 1, 1);
    widget.attach(_gridOffsetSpinButtons.ySpin, 3, 4, 1, 1);

    widget.attach(_gridPaddingLabel, 0, 5, 1, 1);
    widget.attach(_gridPaddingSpinButtons.widthSpin, 1, 5, 1, 1);
    widget.attach(_gridPaddingCrossLabel, 2, 5, 1, 1);
    widget.attach(_gridPaddingSpinButtons.heightSpin, 3, 5, 1, 1);

    widget.attach(_gridOriginLabel, 0, 6, 1, 1);
    widget.attach(_gridOriginSpinButtons.xSpin, 1, 6, 1, 1);
    widget.attach(_gridOriginCommaLabel, 2, 6, 1, 1);
    widget.attach(_gridOriginSpinButtons.ySpin, 3, 6, 1, 1);

    updateGuiValues();

    /**
     * SLOTS
     */

    _selection.signal_selectTransparentModeChanged.connect([this](void) {
        _transparentColorButton.set_active(_selection.selectTransparentMode());
    });

    /* Set Parameter has finished editing */
    // signal_editing_done does not work
    // using activate and focus out instead.
    _nameEntry.signal_activate().connect([this](void) {
        if (_frameSet && !_updatingValues) {
            frameSet_setName(_frameSet, _nameEntry.get_text());
        }
    });
    _nameEntry.signal_focus_out_event().connect([this](GdkEventFocus*) {
        if (_frameSet && !_updatingValues) {
            frameSet_setName(_frameSet, _nameEntry.get_text());
        }
        return false;
    });

    _imageFilenameButton.signal_clicked().connect(
        sigc::mem_fun(*this, &FrameSetPropertiesEditor::on_imageFilenameButtonClicked));

    _gridFrameSizeSpinButtons.signal_valueChanged.connect([this](void) {
        if (_frameSet && !_updatingValues) {
            frameSet_Grid_setFrameSize(_frameSet, _gridFrameSizeSpinButtons.value());
        }
    });

    _gridOffsetSpinButtons.signal_valueChanged.connect([this](void) {
        if (_frameSet && !_updatingValues) {
            frameSet_Grid_setOffset(_frameSet, _gridOffsetSpinButtons.value());
        }
    });

    _gridPaddingSpinButtons.signal_valueChanged.connect([this](void) {
        if (_frameSet && !_updatingValues) {
            frameSet_Grid_setPadding(_frameSet, _gridPaddingSpinButtons.value());
        }
    });

    _gridOriginSpinButtons.signal_valueChanged.connect([this](void) {
        if (_frameSet && !_updatingValues) {
            frameSet_Grid_setOrigin(_frameSet, _gridOriginSpinButtons.value());
        }
    });

    _transparentColorButton.signal_toggled().connect([this](void) {
        _selection.setSelectTransparentMode(_transparentColorButton.get_active());
    });

    /** FrameSet Updated signal */
    Signals::frameSetChanged.connect([this](const SI::FrameSet* frameSet) {
        if (_frameSet == frameSet) {
            updateGuiValues();
        }
    });
}

void FrameSetPropertiesEditor::updateGuiValues()
{
    if (_frameSet) {
        _updatingValues = true;

        _nameEntry.set_text(_frameSet->name());

        _imageFilenameEntry.set_text(_frameSet->imageFilename());
        _imageFilenameEntry.set_position(-1); // right align text

        if (_frameSet->transparentColorValid()) {
            auto transparent = _frameSet->transparentColor();

            auto c = Glib::ustring::format(std::setfill(L'0'), std::setw(6), std::hex,
                                           transparent.rgb());
            _transparentColorEntry.set_text(c);

            _transparentColorButton.set_color(transparent);
        }
        else {
            _transparentColorEntry.set_text("");
            _transparentColorButton.unset_color();
        }

        _gridFrameSizeSpinButtons.set_value(_frameSet->grid().frameSize());
        _gridOffsetSpinButtons.set_value(_frameSet->grid().offset());
        _gridPaddingSpinButtons.set_value(_frameSet->grid().padding());
        _gridOriginSpinButtons.set_value(_frameSet->grid().origin());

        _updatingValues = false;

        widget.set_sensitive(true);
    }
    else {
        static const upoint zeroPoint(0, 0);
        static const usize zeroSize(0, 0);

        _nameEntry.set_text("");
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
    if (!_frameSet || _updatingValues) {
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

    if (!_frameSet->imageFilename().empty()) {
        dialog.set_filename(_frameSet->imageFilename());
    }

    // Set transient parent from widget
    // Could not derefernce RefPtr<Gtk::Window> for base-class constructor
    // using C method instead
    GtkWidget* toplevel = gtk_widget_get_toplevel(GTK_WIDGET(widget.gobj()));
    gtk_window_set_transient_for(GTK_WINDOW(dialog.gobj()), GTK_WINDOW(toplevel));

    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK) {
        frameSet_setImageFilename(_frameSet, dialog.get_filename());
    }
}
