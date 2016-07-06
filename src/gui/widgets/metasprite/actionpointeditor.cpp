#include "actionpointeditor.h"
#include "gui/widgets/defaults.h"
#include "models/common/string.h"

#include <glibmm/i18n.h>

using namespace UnTech::Widgets::MetaSprite;
typedef UnTech::Controller::BaseController BaseController;

ActionPointEditor::ActionPointEditor(MS::ActionPointController& controller)
    : widget()
    , _controller(controller)
    , _locationSpinButtons()
    , _parameterEntry()
    , _locationLabel(_("Location:"), Gtk::ALIGN_START)
    , _locationCommaLabel(" ,  ")
    , _parameterLabel(_("Parameter:"), Gtk::ALIGN_START)
    , _updatingValues(false)
{
    widget.set_row_spacing(DEFAULT_ROW_SPACING);

    widget.attach(_locationLabel, 0, 0, 1, 1);
    widget.attach(_locationSpinButtons.xSpin, 1, 0, 1, 1);
    widget.attach(_locationCommaLabel, 2, 0, 1, 1);
    widget.attach(_locationSpinButtons.ySpin, 3, 0, 1, 1);

    widget.attach(_parameterLabel, 0, 1, 1, 1);
    widget.attach(_parameterEntry, 1, 1, 3, 1);

    updateGuiValues();

    /*
     * SLOTS
     * =====
     */

    /* Controller signals */
    _controller.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &ActionPointEditor::updateGuiValues));

    _controller.signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &ActionPointEditor::updateGuiValues));

    /** Set location signal */
    _locationSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            _controller.selected_setLocation_merge(_locationSpinButtons.value());
        }
    });
    _locationSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _controller.baseController(), &BaseController::dontMergeNextAction)));

    /* Set Parameter has finished editing */
    // signal_editing_done does not work
    // using activate and focus out instead.
    _parameterEntry.signal_activate().connect(sigc::mem_fun(
        *this, &ActionPointEditor::onParameterFinishedEditing));
    _parameterEntry.signal_focus_out_event().connect([this](GdkEventFocus*) {
        this->onParameterFinishedEditing();
        return false;
    });
}

void ActionPointEditor::updateGuiValues()
{
    const MS::ActionPoint* actionPoint = _controller.selected();

    if (actionPoint) {
        _updatingValues = true;

        _locationSpinButtons.set_value(actionPoint->location());
        _parameterEntry.set_text(Glib::ustring::compose("%1", actionPoint->parameter()));

        _updatingValues = false;

        widget.set_sensitive(true);
    }
    else {
        static const ms8point zeroPoint(0, 0);

        _locationSpinButtons.set_value(zeroPoint);
        _parameterEntry.set_text("");

        widget.set_sensitive(false);
    }
}

void ActionPointEditor::onParameterFinishedEditing()
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
