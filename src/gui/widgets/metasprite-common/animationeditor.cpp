#include "animationeditor.h"
#include "gui/widgets/defaults.h"
#include "models/common/string.h"

#include <glibmm/i18n.h>
#include <iomanip>

using namespace UnTech::Widgets::MetaSpriteCommon;

AnimationEditor::AnimationEditor(MSC::AbstractFrameSetController& controller)
    : widget(Gtk::ORIENTATION_VERTICAL)
    , _controller(controller)
    , _animationListEditor(controller.animationController())
    , _instructionListEditor(controller.animationInstructionController())
    , _instructionsFrame(_("Animation Bytecode"))
    , _instructionsBox(Gtk::ORIENTATION_VERTICAL)
    , _instructionsGrid()
    , _operationCombo()
    , _frameNameEntry()
    , _frameFlipCombo()
    , _parameterEntry()
    , _parameterMeaning("", Gtk::ALIGN_END)
    , _operationLabel(_("Operation:"), Gtk::ALIGN_START)
    , _frameLabel(_("Frame:"), Gtk::ALIGN_START)
    , _parameterLabel(_("Parameter:"), Gtk::ALIGN_START)
    , _updatingValues(false)
{
    _instructionsFrame.set_border_width(DEFAULT_BORDER);
    _instructionsBox.set_border_width(DEFAULT_BORDER);
    _instructionsGrid.set_border_width(DEFAULT_BORDER);
    _instructionsGrid.set_row_spacing(DEFAULT_ROW_SPACING);

    _frameFlipCombo.append(" ");
    _frameFlipCombo.append("H Flip");
    _frameFlipCombo.append("V Flip");
    _frameFlipCombo.append("HV Flip");

    _instructionsGrid.attach(_operationLabel, 0, 0, 1, 1);
    _instructionsGrid.attach(_operationCombo, 1, 0, 2, 1);

    _instructionsGrid.attach(_frameLabel, 0, 1, 1, 1);
    _instructionsGrid.attach(_frameNameEntry, 1, 1, 1, 1);
    _instructionsGrid.attach(_frameFlipCombo, 2, 1, 1, 1);

    _instructionsGrid.attach(_parameterLabel, 0, 2, 1, 1);
    _instructionsGrid.attach(_parameterEntry, 1, 2, 1, 1);
    _instructionsGrid.attach(_parameterMeaning, 2, 2, 1, 1);

    _instructionsBox.pack_start(_instructionListEditor.widget, true, true);
    _instructionsBox.pack_start(_instructionsGrid, false, false);

    _instructionsFrame.add(_instructionsBox);

    widget.pack_start(_animationListEditor.widget, true, true);
    widget.pack_start(_instructionsFrame, true, true);

    updateGuiValues();

    /*
     * SLOTS
     * =====
     */

    /** Controller signals */
    _controller.animationInstructionController().signal_selectedChanged().connect(sigc::mem_fun(
        *this, &AnimationEditor::updateGuiValues));

    _controller.animationInstructionController().signal_selectedDataChanged().connect(sigc::mem_fun(
        *this, &AnimationEditor::updateGuiValues));

    /** GUI Signals */

    _operationCombo.signal_changed().connect([this](void) {
        if (!_updatingValues) {
            _controller.animationInstructionController().selected_setOperation(_operationCombo.get_value());
        }
    });

    // signal_editing_done does not work
    // using activate and focus out instead.
    _frameNameEntry.signal_activate().connect(sigc::mem_fun(
        *this, &AnimationEditor::instruction_setFrame));

    _frameNameEntry.signal_focus_out_event().connect([this](GdkEventFocus*) {
        instruction_setFrame();
        return false;
    });

    _frameFlipCombo.signal_changed().connect(sigc::mem_fun(
        *this, &AnimationEditor::instruction_setFrame));

    // signal_editing_done does not work
    // using activate and focus out instead.
    _parameterEntry.signal_activate().connect(sigc::mem_fun(
        *this, &AnimationEditor::instruction_setParameter));
    _parameterEntry.signal_focus_out_event().connect([this](GdkEventFocus*) {
        instruction_setParameter();
        return false;
    });
}

void AnimationEditor::updateGuiValues()
{
    const MSC::AnimationInstruction* inst = _controller.animationInstructionController().selected();

    if (inst) {
        typedef MSC::AnimationBytecode::Enum BC;

        const MSC::AnimationBytecode& op = inst->operation();

        _updatingValues = true;

        _operationCombo.set_value(op);
        _operationLabel.set_sensitive(true);

        if (op.usesFrame()) {
            const auto& fref = inst->frame();

            _frameLabel.set_sensitive(true);

            _frameNameEntry.set_text(fref.frameName);
            _frameNameEntry.set_sensitive(true);

            int index = (fref.vFlip << 1) | fref.hFlip;
            _frameFlipCombo.set_active(index);
            _frameFlipCombo.set_sensitive(true);
        }
        else {
            _frameLabel.set_sensitive(false);
            _frameNameEntry.set_text("");
            _frameNameEntry.set_sensitive(false);
            _frameFlipCombo.unset_active();
            _frameFlipCombo.set_sensitive(false);
        }

        if (op.usesParameter()) {
            _parameterLabel.set_sensitive(true);

            _parameterEntry.set_text(Glib::ustring::format(inst->parameter()));
            _parameterEntry.set_sensitive(true);

            switch (op.value()) {
            case BC::SET_FRAME_AND_WAIT_FRAMES:
                _parameterMeaning.set_text(_("frames"));
                break;

            case BC::SET_FRAME_AND_WAIT_TIME:
                _parameterMeaning.set_text(
                    Glib::ustring::compose("%1 ms",
                                           inst->parameter() * 1000 / 75));
                break;

            case BC::SET_FRAME_AND_WAIT_XVECL:
            case BC::SET_FRAME_AND_WAIT_YVECL:
                _parameterMeaning.set_text(
                    Glib::ustring::format(std::setprecision(3),
                                          (double)inst->parameter() / 32)
                    + " px");
                break;

            default:
                break;
            }

            _parameterMeaning.set_sensitive(true);
        }
        else if (op.usesGotoLabel()) {
            _parameterLabel.set_sensitive(true);

            _parameterEntry.set_text(inst->gotoLabel());
            _parameterEntry.set_sensitive(true);

            _parameterMeaning.set_text("");
            _parameterMeaning.set_sensitive(true);
        }
        else {
            _parameterLabel.set_sensitive(false);

            _parameterEntry.set_text("");
            _parameterEntry.set_sensitive(false);

            _parameterMeaning.set_text("");
            _parameterMeaning.set_sensitive(false);
        }

        _updatingValues = false;
    }
    else {
        _operationLabel.set_sensitive(false);
        _operationCombo.unset_active();

        _frameLabel.set_sensitive(false);
        _frameNameEntry.set_text("");
        _frameFlipCombo.unset_active();

        _parameterLabel.set_sensitive(false);
        _parameterEntry.set_text("");
        _parameterMeaning.set_text("");
    }
}

void AnimationEditor::instruction_setFrame()
{
    const MSC::AnimationInstruction* inst = _controller.animationInstructionController().selected();

    if (inst && !_updatingValues) {
        const MSC::AnimationBytecode& op = inst->operation();

        if (op.usesFrame()) {
            MSC::FrameReference ref;

            ref.frameName = _frameNameEntry.get_text();
            ref.hFlip = _frameFlipCombo.get_active_row_number() & 0x01;
            ref.vFlip = (_frameFlipCombo.get_active_row_number() >> 1) & 0x01;

            _controller.animationInstructionController().selected_setFrame(ref);
        }
    }
}

void AnimationEditor::instruction_setParameter()
{
    const MSC::AnimationInstruction* inst = _controller.animationInstructionController().selected();

    if (inst && !_updatingValues) {
        const MSC::AnimationBytecode& op = inst->operation();

        if (op.usesParameter()) {
            auto value = UnTech::String::toInt(_parameterEntry.get_text());
            if (value.second) {
                _controller.animationInstructionController().selected_setParameter(value.first);
            }
        }
        else if (op.usesGotoLabel()) {
            _controller.animationInstructionController().selected_setGotoLabel(_parameterEntry.get_text());
        }
        else {
            updateGuiValues();
        }
    }
}
