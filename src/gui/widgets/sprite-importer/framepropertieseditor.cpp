#include "framepropertieseditor.h"
#include "gui/widgets/defaults.h"
#include "models/common/string.h"

#include <glibmm/i18n.h>

using namespace UnTech::Widgets::SpriteImporter;

FramePropertiesEditor::FramePropertiesEditor(SI::SpriteImporterController& controller)
    : widget()
    , _controller(controller)
    , _gridLocationSpinButtons()
    , _locationSpinButtons()
    , _originSpinButtons()
    , _tileHitboxSpinButtons()
    , _spriteOrderAdjustment(Gtk::Adjustment::create(2.0, 0.0, 3.0, 1.0, 1.0, 0.0))
    , _spriteOrderSpinButton(_spriteOrderAdjustment)
    , _useGridLocationCB(_("Use Grid Location"))
    , _useCustomOriginCB(_("Use Custom Origin"))
    , _solidCB(_("Solid Tile Hitbox"))
    , _emptySpace()
    , _gridLocationLabel(_("Grid Location:"), Gtk::ALIGN_START)
    , _gridLocationCommaLabel(" ,  ")
    , _locationLabel(_("Location:"), Gtk::ALIGN_START)
    , _locationCommaLabel(" , ")
    , _locationCrossLabel(" x ")
    , _originLabel(_("Origin:"), Gtk::ALIGN_START)
    , _originCommaLabel(" , ")
    , _tileHitboxLabel(_("Tile Hitbox:"), Gtk::ALIGN_START)
    , _tileHitboxCommaLabel(" , ")
    , _tileHitboxCrossLabel(" x ")
    , _spriteOrderLabel(_("Sprite Order:"))
    , _updatingValues(false)
{
    _emptySpace.set_size_request(GRID_CHILD_MARGIN_WIDTH, 2);

    widget.set_row_spacing(DEFAULT_ROW_SPACING);

    widget.attach(_spriteOrderLabel, 0, 0, 2, 1);
    widget.attach(_spriteOrderSpinButton, 2, 0, 3, 1);

    widget.attach(_useGridLocationCB, 0, 1, 5, 1);

    widget.attach(_emptySpace, 0, 2, 1, 1);
    widget.attach(_gridLocationLabel, 1, 2, 1, 1);
    widget.attach(_gridLocationSpinButtons.xSpin, 2, 2, 1, 1);
    widget.attach(_gridLocationCommaLabel, 3, 2, 1, 1);
    widget.attach(_gridLocationSpinButtons.ySpin, 4, 2, 1, 1);

    widget.attach(_locationLabel, 1, 3, 1, 1);
    widget.attach(_locationSpinButtons.xSpin, 2, 3, 1, 1);
    widget.attach(_locationCommaLabel, 3, 3, 1, 1);
    widget.attach(_locationSpinButtons.ySpin, 4, 3, 1, 1);
    widget.attach(_locationSpinButtons.widthSpin, 2, 4, 1, 1);
    widget.attach(_locationCrossLabel, 3, 4, 1, 1);
    widget.attach(_locationSpinButtons.heightSpin, 4, 4, 1, 1);

    widget.attach(_useCustomOriginCB, 0, 5, 5, 1);

    widget.attach(_originLabel, 1, 6, 1, 1);
    widget.attach(_originSpinButtons.xSpin, 2, 6, 1, 1);
    widget.attach(_originCommaLabel, 3, 6, 1, 1);
    widget.attach(_originSpinButtons.ySpin, 4, 6, 1, 1);

    widget.attach(_solidCB, 0, 7, 5, 1);

    widget.attach(_tileHitboxLabel, 1, 8, 1, 1);
    widget.attach(_tileHitboxSpinButtons.xSpin, 2, 8, 1, 1);
    widget.attach(_tileHitboxCommaLabel, 3, 8, 1, 1);
    widget.attach(_tileHitboxSpinButtons.ySpin, 4, 8, 1, 1);
    widget.attach(_tileHitboxSpinButtons.widthSpin, 2, 9, 1, 1);
    widget.attach(_tileHitboxCrossLabel, 3, 9, 1, 1);
    widget.attach(_tileHitboxSpinButtons.heightSpin, 4, 9, 1, 1);

    updateGuiValues();

    /**
     * SLOTS
     */

    // Controller Signals
    _controller.frameController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FramePropertiesEditor::updateGuiValues));

    _controller.frameController().signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &FramePropertiesEditor::updateGuiValues));

    _controller.frameSetController().signal_gridChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &FramePropertiesEditor::updateGuiValues)));

    /** Grid location signal */
    _gridLocationSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            _controller.frameController().selected_setGridLocation_merge(
                _gridLocationSpinButtons.value());
        }
    });
    _gridLocationSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _controller, &SI::SpriteImporterController::dontMergeNextAction)));

    /** Location signal */
    _locationSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            _controller.frameController().selected_setLocation_merge(
                _locationSpinButtons.value());
        }
    });
    _locationSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _controller, &SI::SpriteImporterController::dontMergeNextAction)));

    /** Origin signal */
    _originSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            _controller.frameController().selected_setOrigin_merge(
                _originSpinButtons.value());
        }
    });
    _originSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _controller, &SI::SpriteImporterController::dontMergeNextAction)));

    /** Tile hitbox signal */
    _tileHitboxSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            _controller.frameController().selected_setTileHitbox_merge(
                _tileHitboxSpinButtons.value());
        }
    });
    _tileHitboxSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _controller, &SI::SpriteImporterController::dontMergeNextAction)));

    /** Sprite Order Signal */
    _spriteOrderSpinButton.signal_value_changed().connect([this](void) {
        int i = _spriteOrderSpinButton.get_value_as_int();
        assert(i >= 0);
        assert(i <= 3);

        if (!_updatingValues) {
            _controller.frameController().selected_setSpriteOrder((unsigned)i);
        }
    });

    /** Use Grid Location Checkbox signal */
    _useGridLocationCB.signal_clicked().connect([this](void) {
        if (!_updatingValues) {
            _controller.frameController().selected_setUseGridLocation(
                _useGridLocationCB.get_active());
        }
    });

    /** Use Custom Origin Checkbox signal */
    _useCustomOriginCB.signal_clicked().connect([this](void) {
        if (!_updatingValues) {
            _controller.frameController().selected_setUseGridOrigin(
                !(_useCustomOriginCB.get_active()));
        }
    });

    /** Solid Checkbox signal */
    _solidCB.signal_clicked().connect([this](void) {
        if (!_updatingValues) {
            _controller.frameController().selected_setSolid(
                _solidCB.get_active());
        }
    });
}

void FramePropertiesEditor::updateGuiValues()
{
    const SI::Frame* frame = _controller.frameController().selected();

    if (frame) {
        _updatingValues = true;

        _gridLocationSpinButtons.set_value(frame->gridLocation());
        _locationSpinButtons.set_value(frame->location());
        _originSpinButtons.set_value(frame->origin());
        _spriteOrderSpinButton.set_value(frame->spriteOrder());
        _tileHitboxSpinButtons.set_value(frame->tileHitbox());

        // ::TODO _gridLocationSpinButtons range based on image size::
        // ::TODO _locationSpinButtons range based on image size::
        _locationSpinButtons.set_minSize(frame->minimumViableSize());
        _originSpinButtons.set_range(frame->locationSize());
        _tileHitboxSpinButtons.set_range(frame->locationSize());

        bool useGridLocation = frame->useGridLocation();
        _useGridLocationCB.set_active(useGridLocation);

        _gridLocationSpinButtons.set_sensitive(useGridLocation);
        _gridLocationLabel.set_sensitive(useGridLocation);
        _gridLocationCommaLabel.set_sensitive(useGridLocation);

        _locationSpinButtons.set_sensitive(!useGridLocation);
        _locationLabel.set_sensitive(!useGridLocation);
        _locationCommaLabel.set_sensitive(!useGridLocation);
        _locationCrossLabel.set_sensitive(!useGridLocation);

        bool useCustomOrigin = !frame->useGridOrigin();
        _useCustomOriginCB.set_active(useCustomOrigin);

        _originSpinButtons.set_sensitive(useCustomOrigin);
        _originLabel.set_sensitive(useCustomOrigin);
        _originCommaLabel.set_sensitive(useCustomOrigin);

        bool solid = frame->solid();
        _solidCB.set_active(solid);

        _tileHitboxSpinButtons.set_sensitive(solid);
        _tileHitboxLabel.set_sensitive(solid);
        _tileHitboxCommaLabel.set_sensitive(solid);
        _tileHitboxCrossLabel.set_sensitive(solid);

        _updatingValues = false;

        widget.set_sensitive(true);
    }
    else {
        static const upoint zeroPoint(0, 0);
        static const urect zeroRect(0, 0, 0, 0);

        _gridLocationSpinButtons.set_value(zeroPoint);
        _locationSpinButtons.set_value(zeroRect);
        _originSpinButtons.set_value(zeroPoint);
        _spriteOrderSpinButton.set_value(0);
        _tileHitboxSpinButtons.set_value(zeroRect);

        _useGridLocationCB.set_active(false);
        _useCustomOriginCB.set_active(false);
        _solidCB.set_active(false);

        widget.set_sensitive(false);
    }
}
