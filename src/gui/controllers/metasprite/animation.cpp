#include "animation.h"

using namespace UnTech::MetaSprite::Animation;

const std::string AnimationController::HUMAN_TYPE_NAME = "Animation";
const std::string InstructionController::HUMAN_TYPE_NAME = "Animation Instruction";

// InstructionController
// ---------------------

void InstructionController::selected_setOperation(const Bytecode bc)
{
    edit_selected([&](Instruction& inst) {
        inst.operation = bc;
    });
}

void InstructionController::selected_setParameter(int parameter)
{
    edit_selected([&](Instruction& inst) {
        inst.parameter = parameter;
    });
}

void InstructionController::selected_setFrame(const NameReference& frame)
{
    edit_selected([&](Instruction& inst) {
        inst.frame = frame;
    });
}

void InstructionController::selected_setGotoLabel(const idstring& label)
{
    edit_selected([&](Instruction& inst) {
        inst.gotoLabel = label;
    });
}
