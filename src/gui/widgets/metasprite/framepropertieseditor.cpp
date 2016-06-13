#include "framepropertieseditor.h"
#include "gui/widgets/defaults.h"

#include <glibmm/i18n.h>

using namespace UnTech::Widgets::MetaSprite;
typedef UnTech::Controller::BaseController BaseController;

FramePropertiesEditor::FramePropertiesEditor(MS::FrameController& controller)
    : widget()
    , _controller(controller)
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

    /* Controller signals */
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &FramePropertiesEditor::updateGuiValues));

    _controller.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &FramePropertiesEditor::updateGuiValues));

    /** Tile hitbox signal */
    _tileHitboxSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            _controller.selected_setTileHitbox_merge(_tileHitboxSpinButtons.value());
        }
    });
    _tileHitboxSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _controller.baseController(), &BaseController::dontMergeNextAction)));

    /** Solid Checkbox signal */
    _solidCB.signal_clicked().connect([this](void) {
        if (!_updatingValues) {
            _controller.selected_setSolid(_solidCB.get_active());
        }
    });
}

void FramePropertiesEditor::updateGuiValues()
{
    const MS::Frame* frame = _controller.selected();

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
