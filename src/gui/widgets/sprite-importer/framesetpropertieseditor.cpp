#include "framesetpropertieseditor.h"

using namespace UnTech::Widgets::SpriteImporter;
namespace SI = UnTech::SpriteImporter;

FrameSetPropertiesEditor::FrameSetPropertiesEditor()
    : widget()
    , _frameSet(nullptr)
    , _imageFilenameBox(Gtk::ORIENTATION_HORIZONTAL)
    , _imageFilenameEntry()
    , _imageFilenameButton(_("..."))
    , _gridFrameSizeSpinButtons()
    , _gridOffsetSpinButtons()
    , _gridPaddingSpinButtons()
    , _gridOriginSpinButtons()
    , _imageFilenameLabel(_("Image:"), Gtk::ALIGN_START)
    , _gridFrameSizeLabel(_("Grid Size:"), Gtk::ALIGN_START)
    , _gridFrameSizeCrossLabel(_(" x "))
    , _gridOffsetLabel(_("Grid Offset:"), Gtk::ALIGN_START)
    , _gridOffsetCommaLabel(_(" , "))
    , _gridPaddingLabel(_("Grid Padding:"), Gtk::ALIGN_START)
    , _gridPaddingCrossLabel(_(" x "))
    , _gridOriginLabel(_("Grid Origin:"), Gtk::ALIGN_START)
    , _gridOriginCommaLabel(_(" , "))
{
    widget.set_row_spacing(DEFAULT_ROW_SPACING);

    _imageFilenameEntry.set_sensitive(false);

    _imageFilenameBox.add(_imageFilenameEntry);
    _imageFilenameBox.add(_imageFilenameButton);

    widget.attach(_imageFilenameLabel, 0, 0, 1, 1);
    widget.attach(_imageFilenameBox, 1, 0, 3, 1);

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

    _imageFilenameButton.signal_clicked().connect([this](void) {
        if (_frameSet) {
            // ::TODO implement::
            signal_frameSetChanged.emit(_frameSet);
        }
    });

    _gridFrameSizeSpinButtons.signal_valueChanged.connect([this](void) {
        if (_frameSet) {
            _frameSet->grid().setFrameSize(_gridFrameSizeSpinButtons.value());
            signal_frameSetGridChanged.emit(_frameSet);
            signal_frameSetChanged.emit(_frameSet);
        }
    });

    _gridOffsetSpinButtons.signal_valueChanged.connect([this](void) {
        if (_frameSet) {
            _frameSet->grid().setOffset(_gridOffsetSpinButtons.value());
            signal_frameSetChanged.emit(_frameSet);
            signal_frameSetGridChanged.emit(_frameSet);
        }
    });

    _gridPaddingSpinButtons.signal_valueChanged.connect([this](void) {
        if (_frameSet) {
            _frameSet->grid().setPadding(_gridPaddingSpinButtons.value());
            signal_frameSetChanged.emit(_frameSet);
            signal_frameSetGridChanged.emit(_frameSet);
        }
    });

    _gridOriginSpinButtons.signal_valueChanged.connect([this](void) {
        if (_frameSet) {
            _frameSet->grid().setOrigin(_gridOriginSpinButtons.value());
            signal_frameSetChanged.emit(_frameSet);
            signal_frameSetGridChanged.emit(_frameSet);
        }
    });

    /** FrameSet Updated signal */
    signal_frameSetChanged.connect([this](const std::shared_ptr<SI::FrameSet> frameSet) {
        if (_frameSet == frameSet) {
            updateGuiValues();
        }
    });
}

void FrameSetPropertiesEditor::updateGuiValues()
{
    if (_frameSet) {
        _imageFilenameEntry.set_text(_frameSet->imageFilename());

        _gridFrameSizeSpinButtons.set_value(_frameSet->grid().frameSize());
        _gridOffsetSpinButtons.set_value(_frameSet->grid().offset());
        _gridPaddingSpinButtons.set_value(_frameSet->grid().padding());
        _gridOriginSpinButtons.set_value(_frameSet->grid().origin());

        widget.set_sensitive(true);
    }
    else {
        static const upoint zeroPoint = { 0, 0 };
        static const usize zeroSize = { 0, 0 };

        _imageFilenameEntry.set_text("");

        _gridFrameSizeSpinButtons.set_value(zeroSize);
        _gridOffsetSpinButtons.set_value(zeroPoint);
        _gridPaddingSpinButtons.set_value(zeroSize);
        _gridOriginSpinButtons.set_value(zeroPoint);

        widget.set_sensitive(false);
    }
}
