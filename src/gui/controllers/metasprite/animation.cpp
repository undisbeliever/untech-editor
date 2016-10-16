#include "animation.h"
#include "metasprite.h"
#include "spriteimporter.h"
#include "gui/controllers/containers/cappedvectorcontroller.hpp"
#include "gui/controllers/containers/idmapcontroller.hpp"

using namespace UnTech;
using namespace UnTech::MetaSprite::Animation;
using UndoActionType = UnTech::Controller::Undo::ActionType;

namespace MS = UnTech::MetaSprite::MetaSprite;
namespace SI = UnTech::MetaSprite::SpriteImporter;

namespace UnTech {
namespace Controller {
template <>
Animation::map_t& idmapFromParent<Animation, Animation::map_t>(Animation::map_t& am)
{
    return am;
}

template <>
Instruction::list_t& listFromParent<Instruction::list_t, Animation>(Animation& a)
{
    return a.instructions;
}
}
}

// AnimationControllerInterface
// ----------------------------
AnimationControllerInterface::element_type&
AnimationControllerInterface::elementFromUndoRef(const UndoRef& ref)
{
    if (ref.frameSet) {
        if (ref.type == UndoRef::Type::METASPRITE) {
            auto* a = static_cast<MS::FrameSet*>(ref.frameSet.get());
            return a->animations;
        }
        else if (ref.type == UndoRef::Type::SPRITE_IMPORTER) {
            auto* a = static_cast<SI::FrameSet*>(ref.frameSet.get());
            return a->animations;
        }
    }

    throw std::logic_error("No root FrameSet");
}

// AnimationControllerImpl
// -----------------------
namespace UnTech {
namespace MetaSprite {
namespace Animation {

template class AnimationControllerImpl<MS::FrameSetController>;
template class AnimationControllerImpl<SI::FrameSetController>;

template <class FSC>
typename AnimationControllerImpl<FSC>::element_type*
AnimationControllerImpl<FSC>::editable_selected()
{
    auto* fs = _parent.editable_selected();
    return fs ? &fs->animations : nullptr;
}

template <>
AnimationControllerImpl<MS::FrameSetController>::UndoRef
AnimationControllerImpl<MS::FrameSetController>::undoRefForSelected() const
{
    return {
        std::static_pointer_cast<void>(_parent.getRoot()),
        UndoRef::Type::METASPRITE
    };
}

template <>
AnimationControllerImpl<SI::FrameSetController>::UndoRef
AnimationControllerImpl<SI::FrameSetController>::undoRefForSelected() const
{
    return {
        std::static_pointer_cast<void>(_parent.getRoot()),
        UndoRef::Type::SPRITE_IMPORTER
    };
}
}
}
}

// AnimationController
// -------------------
template class Controller::IdMapController<Animation, AnimationControllerInterface>;

// InstructionController
// ---------------------
template class Controller::CappedVectorController<Instruction, Instruction::list_t,
                                                  AnimationController>;

void InstructionController::selected_setOperation(const Bytecode bc)
{
    const static UndoActionType actionType = { "Edit Animation Operation", false };

    edit_selected(
        &actionType,
        [&](auto& inst) { return inst.operation != bc; },
        [&](auto& inst) { inst.operation = bc; });
}

void InstructionController::selected_setParameter(int parameter)
{
    const static UndoActionType actionType = { "Edit Animation Parameter", false };

    edit_selected(
        &actionType,
        [&](auto& inst) { return inst.parameter != parameter; },
        [&](auto& inst) { inst.parameter = parameter; });
}

void InstructionController::selected_setFrame(const NameReference& frame)
{
    const static UndoActionType actionType = { "Edit Animation Frame", false };

    edit_selected(
        &actionType,
        [&](auto& inst) { return inst.frame != frame; },
        [&](auto& inst) { inst.frame = frame; });
}

void InstructionController::selected_setGotoLabel(const idstring& label)
{
    const static UndoActionType actionType = { "Edit Animation Goto Label", false };

    edit_selected(
        &actionType,
        [&](auto& inst) { return inst.gotoLabel != label; },
        [&](auto& inst) { inst.gotoLabel = label; });
}
