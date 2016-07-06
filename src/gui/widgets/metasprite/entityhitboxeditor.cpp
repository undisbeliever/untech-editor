#include "entityhitboxeditor.h"
#include "gui/widgets/defaults.h"
#include "models/common/string.h"

#include <glibmm/i18n.h>

using namespace UnTech::Widgets::MetaSprite;
typedef UnTech::Controller::BaseController BaseController;

EntityHitboxEditor::EntityHitboxEditor(MS::EntityHitboxController& controller)
    : widget()
    , _controller(controller)
    , _aabbSpinButtons()
    , _parameterEntry()
    , _aabbLabel(_("AABB:"), Gtk::ALIGN_START)
    , _aabbCommaLabel(" ,  ")
    , _aabbCrossLabel(" x ")
    , _parameterLabel(_("Parameter:"), Gtk::ALIGN_START)
    , _updatingValues(false)
{
    widget.set_row_spacing(DEFAULT_ROW_SPACING);

    widget.attach(_aabbLabel, 0, 0, 1, 1);
    widget.attach(_aabbSpinButtons.xSpin, 1, 0, 1, 1);
    widget.attach(_aabbCommaLabel, 2, 0, 1, 1);
    widget.attach(_aabbSpinButtons.ySpin, 3, 0, 1, 1);
    widget.attach(_aabbSpinButtons.widthSpin, 1, 1, 1, 1);
    widget.attach(_aabbCrossLabel, 2, 1, 1, 1);
    widget.attach(_aabbSpinButtons.heightSpin, 3, 1, 1, 1);

    widget.attach(_parameterLabel, 0, 2, 1, 1);
    widget.attach(_parameterEntry, 1, 2, 3, 1);

    updateGuiValues();

    /*
     * SLOTS
     * =====
     */

    /* Controller signals */
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &EntityHitboxEditor::updateGuiValues));

    _controller.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &EntityHitboxEditor::updateGuiValues));

    /** Set aabb signal */
    _aabbSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            _controller.selected_setAabb_merge(_aabbSpinButtons.value());
        }
    });
    _aabbSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _controller.baseController(), &BaseController::dontMergeNextAction)));

    /* Set Parameter has finished editing */
    // signal_editing_done does not work
    // using activate and focus out instead.
    _parameterEntry.signal_activate().connect(sigc::mem_fun(
        *this, &EntityHitboxEditor::onParameterFinishedEditing));
    _parameterEntry.signal_focus_out_event().connect([this](GdkEventFocus*) {
        this->onParameterFinishedEditing();
        return false;
    });
}

void EntityHitboxEditor::updateGuiValues()
{
    const MS::EntityHitbox* entityHitbox = _controller.selected();

    if (entityHitbox) {
        _updatingValues = true;

        _aabbSpinButtons.set_value(entityHitbox->aabb());
        _parameterEntry.set_text(Glib::ustring::compose("%1", entityHitbox->parameter()));

        _updatingValues = false;

        widget.set_sensitive(true);
    }
    else {
        static const ms8rect zeroRect(0, 0, 0, 0);

        _aabbSpinButtons.set_value(zeroRect);
        _parameterEntry.set_text("");

        widget.set_sensitive(false);
    }
}

void EntityHitboxEditor::onParameterFinishedEditing()
{
    if (!_updatingValues) {
        auto p = UnTech::String::toUint8(_parameterEntry.get_text());
        if (p) {
            _controller.selected_setParameter(p.value());
        }
        else {
            updateGuiValues();
        }
    }
}
