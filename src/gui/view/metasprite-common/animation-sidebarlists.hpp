#pragma once

#include "gui/controllers/metasprite-common.h"
#include "gui/view/common/namedlist.h"
#include "gui/view/common/orderedlist.h"

namespace UnTech {
namespace View {

namespace MSC = UnTech::MetaSpriteCommon;

// ANIMATIONS
// ==========

template <>
void NamedListCtrl<MSC::Animation>::CreateColumns()
{
    AppendColumn("Animation", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
}

template <>
wxString NamedListCtrl<MSC::Animation>::OnGetItemText(long item, long column) const
{
    if (item < 0 || (unsigned)item >= _ptrList.size()) {
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
template class OrderedListCtrl<MSC::AnimationInstruction>;

template <>
void OrderedListCtrl<MSC::AnimationInstruction>::CreateColumns()
{
    AppendColumn("Instruction", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
    AppendColumn("Frame", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
    AppendColumn("Parameter", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
}

template <>
wxString OrderedListCtrl<MSC::AnimationInstruction>::OnGetItemText(long item, long column) const
{
    typedef MSC::AnimationBytecode::Enum BC;

    const MSC::AnimationInstruction::list_t* list = _controller.list();

    if (list == nullptr) {
        return wxEmptyString;
    }

    const MSC::AnimationInstruction& inst = list->at(item);
    const MSC::AnimationBytecode& op = inst.operation();

    switch (column) {
    case 0: {
        return op.string();
    }

    case 1: {
        if (op.usesFrame()) {
            const auto& fref = inst.frame();

            wxString ret = fref.frameName;

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
            return wxString::Format("%d", inst.parameter());
            break;

        case BC::SET_FRAME_AND_WAIT_FRAMES:
            return wxString::Format("%d frames", inst.parameter());
            break;

        case BC::SET_FRAME_AND_WAIT_TIME:
            return wxString::Format("%d ms", inst.parameter() * 1000 / 75);

        case BC::SET_FRAME_AND_WAIT_XVECL:
        case BC::SET_FRAME_AND_WAIT_YVECL:
            return wxString::Format("%0.3f px", (double)inst.parameter() / 32);

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
