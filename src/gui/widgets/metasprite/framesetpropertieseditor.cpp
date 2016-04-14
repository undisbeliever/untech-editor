#include "framesetpropertieseditor.h"
#include "gui/undo/actionhelper.h"
#include <iomanip>

using namespace UnTech::Widgets::MetaSprite;

SIMPLE_UNDO_ACTION(frameSet_setName,
                   MS::FrameSet, std::string, name, setName,
                   Signals::frameSetChanged,
                   "Change Name")

FrameSetPropertiesEditor::FrameSetPropertiesEditor()
    : widget()
    , _frameSet(nullptr)
    , _nameEntry()
    , _nameLabel(_("Name:"), Gtk::ALIGN_START)
    , _updatingValues(false)
{
    widget.set_border_width(DEFAULT_BORDER);
    widget.set_row_spacing(DEFAULT_ROW_SPACING);

    widget.attach(_nameLabel, 0, 0, 1, 1);
    widget.attach(_nameEntry, 1, 0, 3, 1);

    updateGuiValues();

    /**
     * SLOTS
     */

    /* Set Parameter has finished editing */
    // signal_editing_done does not work
    // using activate and focus out instead.
    _nameEntry.signal_activate().connect([this](void) {
        if (_frameSet && !_updatingValues) {
            frameSet_setName(_frameSet, _nameEntry.get_text());
        }
    });
    _nameEntry.signal_focus_out_event().connect([this](GdkEventFocus*) {
        if (_frameSet && !_updatingValues) {
            frameSet_setName(_frameSet, _nameEntry.get_text());
        }
        return false;
    });

    /** FrameSet Updated signal */
    Signals::frameSetChanged.connect([this](const std::shared_ptr<MS::FrameSet> frameSet) {
        if (_frameSet == frameSet) {
            updateGuiValues();
        }
    });
}

void FrameSetPropertiesEditor::updateGuiValues()
{
    if (_frameSet) {
        _updatingValues = true;

        _nameEntry.set_text(_frameSet->name());

        _updatingValues = false;

        widget.set_sensitive(true);
    }
    else {

        _nameEntry.set_text("");

        widget.set_sensitive(false);
    }
}
