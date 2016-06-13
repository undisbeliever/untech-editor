#include "entityhitboxeditor.h"
#include "gui/widgets/defaults.h"
#include "models/common/string.h"

using namespace UnTech::Widgets::SpriteImporter;

EntityHitboxEditor::EntityHitboxEditor(SI::SpriteImporterController& controller)
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

    // Controller Signals
    _controller.entityHitboxController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &EntityHitboxEditor::updateGuiValues));

    _controller.entityHitboxController().signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &EntityHitboxEditor::updateGuiValues));

    _controller.frameController().signal_frameSizeChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &EntityHitboxEditor::updateRange)));

    _controller.frameSetController().signal_gridChanged().connect(sigc::hide(sigc::mem_fun(
        *this, &EntityHitboxEditor::updateRange)));

    /** Set aabb signal */
    _aabbSpinButtons.signal_valueChanged.connect([this](void) {
        if (!_updatingValues) {
            _controller.entityHitboxController().selected_setAabb_merge(
                _aabbSpinButtons.value());
        }
    });
    _aabbSpinButtons.signal_focus_out_event.connect(sigc::hide(sigc::mem_fun(
        _controller, &SI::SpriteImporterController::dontMergeNextAction)));

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
    const SI::EntityHitbox* entityHitbox = _controller.entityHitboxController().selected();

    if (entityHitbox) {
        _updatingValues = true;

        _aabbSpinButtons.set_range(entityHitbox->frame().locationSize());
        _aabbSpinButtons.set_value(entityHitbox->aabb());
        _parameterEntry.set_text(Glib::ustring::compose("%1", entityHitbox->parameter()));

        _updatingValues = false;

        widget.set_sensitive(true);
    }
    else {
        static const urect zeroRect(0, 0, 0, 0);

        _aabbSpinButtons.set_value(zeroRect);
        _parameterEntry.set_text("");

        widget.set_sensitive(false);
    }
}

void EntityHitboxEditor::updateRange()
{
    const SI::Frame* frame = _controller.frameController().selected();

    if (frame) {
        _aabbSpinButtons.set_range(frame->locationSize());
    }
}

void EntityHitboxEditor::onParameterFinishedEditing()
{
    if (!_updatingValues) {
        auto value = UnTech::String::toUint8(_parameterEntry.get_text());
        if (value.second) {
            _controller.entityHitboxController().selected_setParameter(value.first);
        }
        else {
            updateGuiValues();
        }
    }
}
