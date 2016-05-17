#include "actionpointeditor.h"
#include "signals.h"
#include "gui/undo/actionhelper.h"
#include "gui/undo/mergeactionhelper.h"
#include "gui/widgets/defaults.h"
#include "models/common/string.h"

#include <glibmm/i18n.h>

using namespace UnTech::Widgets::SpriteImporter;

SIMPLE_UNDO_MERGE_ACTION(actionPoint_merge_setLocation,
                         SI::ActionPoint, UnTech::upoint, location, setLocation,
                         Signals::actionPointChanged,
                         "Move Action Point")

SIMPLE_UNDO_ACTION(actionPoint_setParameter,
                   SI::ActionPoint, unsigned, parameter, setParameter,
                   Signals::actionPointChanged,
                   "Change Action Point Parameter")

ActionPointEditor::ActionPointEditor(Selection& selection)
    : widget()
    , _selection(selection)
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

    /* Update gui when selected actionPoint changed */
    _selection.signal_actionPointChanged.connect(sigc::mem_fun(
        *this, &ActionPointEditor::updateGuiValues));

    /* Update gui if actionPoint has changed */
    Signals::actionPointChanged.connect([this](const SI::ActionPoint* ap) {
        if (ap == _selection.actionPoint()) {
            updateGuiValues();
        }
    });

    /* Update location range if necessary */
    Signals::frameSizeChanged.connect([this](const SI::Frame* frame) {
        if (_selection.actionPoint() && frame == _selection.frame()) {
            _locationSpinButtons.set_range(_selection.frame()->locationSize());
        }
    });

    /* Update location range if necessary */
    Signals::frameSetGridChanged.connect([this](const SI::FrameSet* frameset) {
        if (_selection.actionPoint() && frameset == _selection.frameSet()) {
            _locationSpinButtons.set_range(_selection.frame()->locationSize());
        }
    });

    /** Set location signal */
    _locationSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            actionPoint_merge_setLocation(_selection.actionPoint(),
                                          _locationSpinButtons.value());
        }
    });
    _locationSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _selection, &Selection::dontMergeNextUndoAction)));

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
    const SI::ActionPoint* actionPoint = _selection.actionPoint();

    if (actionPoint) {
        _updatingValues = true;

        _locationSpinButtons.set_range(actionPoint->frame().locationSize());
        _locationSpinButtons.set_value(actionPoint->location());
        _parameterEntry.set_text(Glib::ustring::compose("%1", actionPoint->parameter()));

        _updatingValues = false;

        widget.set_sensitive(true);
    }
    else {
        static const upoint zeroPoint(0, 0);

        _locationSpinButtons.set_value(zeroPoint);
        _parameterEntry.set_text("");

        widget.set_sensitive(false);
    }
}

void ActionPointEditor::onParameterFinishedEditing()
{
    SI::ActionPoint* actionPoint = _selection.actionPoint();

    if (actionPoint && !_updatingValues) {
        auto value = UnTech::String::toUint8(_parameterEntry.get_text());
        if (value.second) {
            actionPoint_setParameter(actionPoint, value.first);
        }
        else {
            updateGuiValues();
        }
    }
}
