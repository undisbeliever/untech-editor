#include "entityhitboxeditor.h"
#include "signals.h"
#include "models/common/string.h"
#include "gui/undo/actionhelper.h"
#include "gui/undo/mergeactionhelper.h"
#include "gui/widgets/defaults.h"

#include <glibmm/i18n.h>

using namespace UnTech::Widgets::MetaSprite;

SIMPLE_UNDO_MERGE_ACTION(entityHitbox_merge_setAabb,
                         MS::EntityHitbox, UnTech::ms8rect, aabb, setAabb,
                         Signals::entityHitboxChanged,
                         "Move Entity Hitbox")

SIMPLE_UNDO_ACTION(entityHitbox_setParameter,
                   MS::EntityHitbox, unsigned, parameter, setParameter,
                   Signals::entityHitboxChanged,
                   "Change Entity Hitbox Parameter")

EntityHitboxEditor::EntityHitboxEditor(Selection& selection)
    : widget()
    , _selection(selection)
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

    /* Update gui when selected entityHitbox changed */
    _selection.signal_entityHitboxChanged.connect(sigc::mem_fun(
        *this, &EntityHitboxEditor::updateGuiValues));

    /* Update gui if entityHitbox has changed */
    Signals::entityHitboxChanged.connect([this](const MS::EntityHitbox* eh) {
        if (eh == _selection.entityHitbox()) {
            updateGuiValues();
        }
    });

    /** Set aabb signal */
    _aabbSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            entityHitbox_merge_setAabb(_selection.entityHitbox(),
                                       _aabbSpinButtons.value());
        }
    });
    _aabbSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _selection, &Selection::dontMergeNextUndoAction)));

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
    const MS::EntityHitbox* entityHitbox = _selection.entityHitbox();

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
    MS::EntityHitbox* entityHitbox = _selection.entityHitbox();

    if (entityHitbox && !_updatingValues) {
        auto value = UnTech::String::toUint8(_parameterEntry.get_text());
        if (value.second) {
            entityHitbox_setParameter(entityHitbox, value.first);
        }
        else {
            updateGuiValues();
        }
    }
}
