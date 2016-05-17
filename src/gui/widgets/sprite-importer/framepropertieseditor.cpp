#include "framepropertieseditor.h"
#include "signals.h"
#include "gui/undo/actionhelper.h"
#include "gui/undo/mergeactionhelper.h"
#include "gui/widgets/defaults.h"
#include "models/common/string.h"

#include <glibmm/i18n.h>

using namespace UnTech::Widgets::SpriteImporter;

// ::SHOULDO add a Boolean type::
// ::: would need undo and redo messages in Undo::Action ::

SIMPLE_UNDO_MERGE_ACTION(frame_merge_setGridLocation,
                         SI::Frame, UnTech::upoint, gridLocation, setGridLocation,
                         Signals::frameChanged,
                         "Change Grid Location")

SIMPLE_UNDO_ACTION2(frame_setUseGridLocation,
                    SI::Frame, bool, useGridLocation, setUseGridLocation,
                    Signals::frameChanged, Signals::frameSizeChanged,
                    "Change Use Grid Location")

SIMPLE_UNDO_MERGE_ACTION2(frame_merge_setLocation,
                          SI::Frame, UnTech::urect, location, setLocation,
                          Signals::frameChanged, Signals::frameSizeChanged,
                          "Change Frame Location")

SIMPLE_UNDO_ACTION(frame_setUseGridOrigin,
                   SI::Frame, bool, useGridOrigin, setUseGridOrigin,
                   Signals::frameChanged,
                   "Change Use Grid Origin")

SIMPLE_UNDO_MERGE_ACTION(frame_merge_setOrigin,
                         SI::Frame, UnTech::upoint, origin, setOrigin,
                         Signals::frameChanged,
                         "Change Frame Origin")

SIMPLE_UNDO_ACTION(frame_setSolid,
                   SI::Frame, bool, solid, setSolid,
                   Signals::frameChanged,
                   "Change Frame Solid")

SIMPLE_UNDO_MERGE_ACTION(frame_merge_setTileHitbox,
                         SI::Frame, UnTech::urect, tileHitbox, setTileHitbox,
                         Signals::frameChanged,
                         "Change Frame Tile Hitbox")

SIMPLE_UNDO_ACTION(frame_setSpriteOrder,
                   SI::Frame, unsigned, spriteOrder, setSpriteOrder,
                   Signals::frameChanged,
                   "Change Frame Sprite Order")

FramePropertiesEditor::FramePropertiesEditor(Selection& selection)
    : widget()
    , _selection(selection)
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

    /* Update gui when selected frame changed */
    _selection.signal_frameChanged.connect(sigc::mem_fun(
        *this, &FramePropertiesEditor::updateGuiValues));

    /* Update gui if frame has changed */
    Signals::frameChanged.connect([this](const SI::Frame* frame) {
        if (frame == _selection.frame()) {
            updateGuiValues();
        }
    });

    /** FrameSet Grid Updated signal */
    Signals::frameSetGridChanged.connect([this](const SI::FrameSet* frameset) {
        if (frameset && frameset == _selection.frameSet()) {
            updateGuiValues();
            Signals::frameSizeChanged.emit(_selection.frame());
        }
    });

    /** Grid location signal */
    _gridLocationSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            frame_merge_setGridLocation(_selection.frame(),
                                        _gridLocationSpinButtons.value());
        }
    });
    _gridLocationSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _selection, &Selection::dontMergeNextUndoAction)));

    /** Location signal */
    _locationSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            frame_merge_setLocation(_selection.frame(),
                                    _locationSpinButtons.value());
        }
    });
    _locationSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _selection, &Selection::dontMergeNextUndoAction)));

    /** Origin signal */
    _originSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            frame_merge_setOrigin(_selection.frame(),
                                  _originSpinButtons.value());
        }
    });
    _originSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _selection, &Selection::dontMergeNextUndoAction)));

    /** Tile hitbox signal */
    _tileHitboxSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            frame_merge_setTileHitbox(_selection.frame(),
                                      _tileHitboxSpinButtons.value());
        }
    });
    _tileHitboxSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _selection, &Selection::dontMergeNextUndoAction)));

    /** Sprite Order Signal */
    _spriteOrderSpinButton.signal_value_changed().connect([this](void) {
        int i = _spriteOrderSpinButton.get_value_as_int();
        assert(i >= 0);
        assert(i <= 3);

        if (!_updatingValues) {
            frame_setSpriteOrder(_selection.frame(), (unsigned)i);
        }
    });

    /** Use Grid Location Checkbox signal */
    _useGridLocationCB.signal_clicked().connect([this](void) {
        if (!_updatingValues) {
            frame_setUseGridLocation(_selection.frame(),
                                     _useGridLocationCB.get_active());
        }
    });

    /** Use Custom Origin Checkbox signal */
    _useCustomOriginCB.signal_clicked().connect([this](void) {
        if (!_updatingValues) {
            frame_setUseGridOrigin(_selection.frame(),
                                   !(_useCustomOriginCB.get_active()));
        }
    });

    /** Solid Checkbox signal */
    _solidCB.signal_clicked().connect([this](void) {
        if (!_updatingValues) {
            frame_setSolid(_selection.frame(),
                           _solidCB.get_active());
        }
    });
}

void FramePropertiesEditor::updateGuiValues()
{
    const SI::Frame* frame = _selection.frame();

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
