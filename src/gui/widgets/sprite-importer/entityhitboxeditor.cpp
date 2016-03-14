#include "entityhitboxeditor.h"

using namespace UnTech::Widgets::SpriteImporter;
namespace SI = UnTech::SpriteImporter;

EntityHitboxEditor::EntityHitboxEditor()
    : widget()
    , _entityHitbox()
    , _aabbSpinButtons()
    , _parameterEntry()
    , _aabbLabel(_("AABB:"), Gtk::ALIGN_START)
    , _aabbCommaLabel(" ,  ")
    , _aabbCrossLabel(" x ")
    , _parameterLabel(_("Parameter:"), Gtk::ALIGN_START)
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

    /** Set aabb signal */
    _aabbSpinButtons.signal_valueChanged.connect([this](void) {
        if (_entityHitbox) {
            _entityHitbox->setAabb(_aabbSpinButtons.value());
            Signals::entityHitboxChanged.emit(_entityHitbox);
            Signals::entityHitboxLocationChanged.emit(_entityHitbox);
        }
    });

    /* Set Parameter has finished editing */
    // signal_editing_done does not work
    // using activate and focus out instead.
    _parameterEntry.signal_activate().connect(sigc::mem_fun(
        *this, &EntityHitboxEditor::onParameterFinishedEditing));
    _parameterEntry.signal_focus_out_event().connect([this](GdkEventFocus*) {
        this->onParameterFinishedEditing();
        return false;
    });

    /* Update gui if object has changed */
    Signals::entityHitboxChanged.connect([this](const std::shared_ptr<SI::EntityHitbox> obj) {
        if (_entityHitbox == obj) {
            updateGuiValues();
        }
    });

    /* Update aabb range if necessary */
    Signals::frameSizeChanged.connect([this](const std::shared_ptr<SI::Frame> frame) {
        if (_entityHitbox) {
            const auto f = _entityHitbox->frame();
            if (frame == f) {
                _aabbSpinButtons.set_range(frame->locationSize());
            }
        }
    });

    Signals::frameSetGridChanged.connect([this](const std::shared_ptr<SI::FrameSet> fs) {
        if (_entityHitbox) {
            const auto frame = _entityHitbox->frame();
            if (frame->frameSet() == fs) {
                _aabbSpinButtons.set_range(frame->locationSize());
            }
        }
    });
}

void EntityHitboxEditor::updateGuiValues()
{
    if (_entityHitbox) {
        auto frame = _entityHitbox->frame();

        if (frame) {
            _aabbSpinButtons.set_range(frame->locationSize());
        }
        _aabbSpinButtons.set_value(_entityHitbox->aabb());
        _parameterEntry.set_text(Glib::ustring::compose("%1", _entityHitbox->parameter()));

        widget.set_sensitive(true);
    }
    else {
        static const urect zeroRect = { 0, 0, 0, 0 };

        _aabbSpinButtons.set_value(zeroRect);
        _parameterEntry.set_text("");

        widget.set_sensitive(false);
    }
}

void EntityHitboxEditor::onParameterFinishedEditing()
{
    if (_entityHitbox) {
        auto value = UnTech::String::toUint8(_parameterEntry.get_text());
        if (value.second) {
            _entityHitbox->setParameter(value.first);
        }
        Signals::entityHitboxChanged.emit(_entityHitbox);
    }
}
