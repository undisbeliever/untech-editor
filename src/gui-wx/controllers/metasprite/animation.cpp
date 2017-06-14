/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animation.h"
#include "metasprite.h"
#include "spriteimporter.h"
#include "gui-wx/controllers/containers/cappedvectorcontroller.hpp"
#include "gui-wx/controllers/containers/idmapcontroller.hpp"

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
AnimationFrame::list_t& listFromParent<AnimationFrame::list_t, Animation>(Animation& a)
{
    return a.frames;
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

void AnimationController::selected_setDurationFormat(DurationFormat format)
{
    const static UndoActionType actionType = { "Edit Animation Duration Format", false };

    edit_selected(
        &actionType,
        [&](auto& ani) { return ani.durationFormat != format; },
        [&](auto& ani) { ani.durationFormat = format; });
}

void AnimationController::selected_setNextAnimation(const idstring& animationId)
{
    const static UndoActionType actionType = { "Edit Next Animation", false };

    edit_selected(
        &actionType,
        [&](auto& ani) { return ani.nextAnimation != animationId; },
        [&](auto& ani) { ani.nextAnimation = animationId; });
}

void AnimationController::selected_setOneShot(bool oneShot)
{
    const static UndoActionType actionType = { "Edit Next Animation", false };

    edit_selected(
        &actionType,
        [&](auto& ani) { return ani.oneShot != oneShot; },
        [&](auto& ani) { ani.oneShot = oneShot; });
}

// AnimationFrameController
// ------------------------
template class Controller::CappedVectorController<AnimationFrame, AnimationFrame::list_t,
                                                  AnimationController>;

void AnimationFrameController::selected_setFrame(const NameReference& frame)
{
    const static UndoActionType actionType = { "Edit Animation Frame", false };

    edit_selected(
        &actionType,
        [&](auto& aFrame) { return aFrame.frame != frame; },
        [&](auto& aFrame) { aFrame.frame = frame; });
}

void AnimationFrameController::selected_setDuration(uint8_t duration)
{
    const static UndoActionType actionType = { "Edit Animation Frame Duration", true };

    edit_selected(
        &actionType,
        [&](auto& aFrame) { return aFrame.duration != duration; },
        [&](auto& aFrame) { aFrame.duration = duration; });
}
