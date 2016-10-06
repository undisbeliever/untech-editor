#include "animation.h"
#include "gui/controllers/containers/cappedvectorcontroller.hpp"
#include "gui/controllers/containers/idmapcontroller.hpp"

using namespace UnTech;
using namespace UnTech::MetaSprite::Animation;

const std::string AnimationController::HUMAN_TYPE_NAME = "Animation";
const std::string InstructionController::HUMAN_TYPE_NAME = "Animation Instruction";

// AnimationController
// -------------------
template class Controller::IdMapController<Animation, AnimationControllerInterface>;

// InstructionController
// ---------------------
template class Controller::CappedVectorController<Instruction, Instruction::list_t,
                                                  AnimationController>;

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
