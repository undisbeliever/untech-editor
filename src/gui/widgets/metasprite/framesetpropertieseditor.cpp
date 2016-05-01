#include "framesetpropertieseditor.h"
#include "document.h"
#include "signals.h"
#include "gui/undo/actionhelper.h"
#include "gui/widgets/defaults.h"

#include <glibmm/i18n.h>

using namespace UnTech::Widgets::MetaSprite;

SIMPLE_UNDO_ACTION(frameSet_setName,
                   MS::FrameSet, std::string, name, setName,
                   Signals::frameSetChanged,
                   "Change Name")

FrameSetPropertiesEditor::FrameSetPropertiesEditor(Selection& selection)
    : widget()
    , _selection(selection)
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

    /** Selected FrameSet changed signal */
    _selection.signal_frameSetChanged.connect(sigc::mem_fun(
        *this, &FrameSetPropertiesEditor::updateGuiValues));

    /** FrameSet Updated signal */
    Signals::frameSetChanged.connect([this](const MS::FrameSet* frameSet) {
        if (frameSet == _selection.frameSet()) {
            updateGuiValues();
        }
    });

    /* Set Parameter has finished editing */
    // signal_editing_done does not work
    // using activate and focus out instead.
    _nameEntry.signal_activate().connect([this](void) {
        if (!_updatingValues) {
            frameSet_setName(_selection.frameSet(),
                             _nameEntry.get_text());
        }
    });
    _nameEntry.signal_focus_out_event().connect([this](GdkEventFocus*) {
        if (!_updatingValues) {
            frameSet_setName(_selection.frameSet(),
                             _nameEntry.get_text());
        }
        return false;
    });
}

void FrameSetPropertiesEditor::updateGuiValues()
{
    const MS::FrameSet* frameSet = _selection.frameSet();

    if (frameSet) {
        _updatingValues = true;

        _nameEntry.set_text(frameSet->name());

        _updatingValues = false;

        widget.set_sensitive(true);
    }
    else {

        _nameEntry.set_text("");

        widget.set_sensitive(false);
    }
}
