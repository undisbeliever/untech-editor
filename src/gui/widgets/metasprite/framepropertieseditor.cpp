#include "framepropertieseditor.h"
#include "signals.h"
#include "gui/undo/actionhelper.h"
#include "gui/undo/mergeactionhelper.h"
#include "gui/widgets/defaults.h"

#include <glibmm/i18n.h>

using namespace UnTech::Widgets::MetaSprite;

// ::SHOULDO add a Boolean type::
// ::: would need undo and redo messages in Undo::Action ::

SIMPLE_UNDO_ACTION(frame_setSolid,
                   MS::Frame, bool, solid, setSolid,
                   Signals::frameChanged,
                   "Change Frame Solid")

SIMPLE_UNDO_MERGE_ACTION(frame_merge_setTileHitbox,
                         MS::Frame, UnTech::ms8rect, tileHitbox, setTileHitbox,
                         Signals::frameChanged,
                         "Change Frame Tile Hitbox")

FramePropertiesEditor::FramePropertiesEditor(Selection& selection)
    : widget()
    , _selection(selection)
    , _tileHitboxSpinButtons()
    , _solidCB(_("Solid Tile Hitbox"))
    , _emptySpace()
    , _tileHitboxLabel(_("Tile Hitbox:"), Gtk::ALIGN_START)
    , _tileHitboxCommaLabel(" , ")
    , _tileHitboxCrossLabel(" x ")
    , _updatingValues(false)
{
    _emptySpace.set_size_request(GRID_CHILD_MARGIN_WIDTH, 2);

    widget.set_row_spacing(DEFAULT_ROW_SPACING);

    // ::SHOULDDO frame name::

    widget.attach(_solidCB, 0, 1, 5, 1);

    widget.attach(_emptySpace, 0, 2, 1, 1);
    widget.attach(_tileHitboxLabel, 1, 2, 1, 1);
    widget.attach(_tileHitboxSpinButtons.xSpin, 2, 2, 1, 1);
    widget.attach(_tileHitboxCommaLabel, 3, 2, 1, 1);
    widget.attach(_tileHitboxSpinButtons.ySpin, 4, 2, 1, 1);
    widget.attach(_tileHitboxSpinButtons.widthSpin, 2, 3, 1, 1);
    widget.attach(_tileHitboxCrossLabel, 3, 3, 1, 1);
    widget.attach(_tileHitboxSpinButtons.heightSpin, 4, 3, 1, 1);

    updateGuiValues();

    /**
     * SLOTS
     */

    /** Selected frame changed signal */
    _selection.signal_frameChanged.connect(sigc::mem_fun(
        *this, &FramePropertiesEditor::updateGuiValues));

    /** Frame Updated signal */
    Signals::frameChanged.connect([this](const MS::Frame* frame) {
        if (frame == _selection.frame()) {
            updateGuiValues();
        }
    });

    /** Tile hitbox signal */
    _tileHitboxSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            frame_merge_setTileHitbox(_selection.frame(),
                                      _tileHitboxSpinButtons.value());
        }
    });
    _tileHitboxSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _selection, &Selection::dontMergeNextUndoAction)));

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
    const MS::Frame* frame = _selection.frame();

    if (frame) {
        _updatingValues = true;

        _tileHitboxSpinButtons.set_value(frame->tileHitbox());

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
        static const ms8rect zeroRect(0, 0, 0, 0);

        _tileHitboxSpinButtons.set_value(zeroRect);

        _solidCB.set_active(false);

        widget.set_sensitive(false);
    }
}
