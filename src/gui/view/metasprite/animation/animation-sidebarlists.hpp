#pragma once

#include "gui/controllers/metasprite/animation.h"
#include "gui/view/common/idmaplistctrl.h"
#include "gui/view/common/vectorlistctrl.h"

namespace UnTech {
namespace View {

namespace MSA = UnTech::MetaSprite::Animation;

// ANIMATIONS
// ==========
template <>
void IdMapListCtrl<MSA::AnimationController>::CreateColumns();

template <>
void IdMapListCtrl<MSA::AnimationController>::CreateColumns()
{
    AppendColumn("Animation", wxLIST_FORMAT_LEFT);

    HideHeader();
    BindColumnsToEqualWidth();
}

template <>
wxString IdMapListCtrl<MSA::AnimationController>::OnGetItemText(long item, long column) const
{
    if (item < 0 || (unsigned)item >= _nameList.size()) {
        return wxEmptyString;
    }

    switch (column) {
    case 0: {
        return _nameList[item];
    }
    default:
        return wxEmptyString;
    }
}

// ANIMATION INSTRUCTIONS
// ======================
template <>
void VectorListCtrl<MSA::InstructionController>::CreateColumns();

template <>
void VectorListCtrl<MSA::InstructionController>::CreateColumns()
{
    AppendColumn("Instruction", wxLIST_FORMAT_LEFT);
    AppendColumn("Frame", wxLIST_FORMAT_LEFT);
    AppendColumn("Parameter", wxLIST_FORMAT_LEFT);

    BindColumnsToEqualWidth();
}

template <>
wxString VectorListCtrl<MSA::InstructionController>::OnGetItemText(long item, long column) const
{
    typedef MSA::Bytecode::Enum BC;

    const MSA::Instruction::list_t* list = _controller.list();

    if (list == nullptr) {
        return wxEmptyString;
    }

    const MSA::Instruction& inst = list->at(item);
    const MSA::Bytecode& op = inst.operation;

    switch (column) {
    case 0: {
        return op.string();
    }

    case 1: {
        if (op.usesFrame()) {
            const auto& fref = inst.frame;

            wxString ret = fref.name.str();

            if (!fref.hFlip) {
                if (fref.vFlip) {
                    ret += " (vFlip)";
                }
            }
            else {
                if (!fref.vFlip) {
                    ret += " (hFlip)";
                }
                else {
                    ret += " (hvFlip)";
                }
            }

            return ret;
        }
        else {
            return wxEmptyString;
        }

    case 2:
        switch (op.value()) {
        case BC::GOTO_OFFSET:
            return wxString::Format("%d", inst.parameter);
            break;

        case BC::GOTO_ANIMATION:
            return inst.gotoLabel.str();
            break;

        case BC::SET_FRAME_AND_WAIT_FRAMES:
            return wxString::Format("%d frames", inst.parameter);
            break;

        case BC::SET_FRAME_AND_WAIT_TIME:
            return wxString::Format("%d ms", inst.parameter * 1000 / 75);

        case BC::SET_FRAME_AND_WAIT_XVECL:
        case BC::SET_FRAME_AND_WAIT_YVECL:
            return wxString::Format("%0.3f px", double(inst.parameter) / 32);

        default:
            return wxEmptyString;
        }
    }

    default:
        return wxEmptyString;
    }
}
}
}
