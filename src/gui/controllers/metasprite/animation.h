#pragma once

#include "../containers/cappedvectorcontroller.h"
#include "../containers/idmapcontroller.h"
#include "models/metasprite/animation/animation.h"

namespace UnTech {
namespace MetaSprite {
namespace Animation {

class AnimationControllerInterface;
class AnimationController;
class InstructionController;

class AnimationController
    : public Controller::IdMapController<Animation, AnimationControllerInterface> {

    friend class InstructionController;

public:
    static const std::string HUMAN_TYPE_NAME;

public:
    AnimationController(AnimationControllerInterface& parent)
        : IdMapController(parent)
    {
    }

protected:
    virtual Animation::map_t* editable_mapFromParent() final;
};

class InstructionController
    : public Controller::CappedVectorController<Instruction, Instruction::list_t,
                                                AnimationController> {

public:
    static const std::string HUMAN_TYPE_NAME;

public:
    InstructionController(AnimationController& parent)
        : CappedVectorController(parent)
    {
    }

    void selected_setOperation(const Bytecode bc);
    void selected_setParameter(int parameter);
    void selected_setFrame(const NameReference& frame);
    void selected_setGotoLabel(const idstring& label);

protected:
    virtual Instruction::list_t* editable_listFromParent() final
    {
        Animation* a = parent().editable_selected();
        return a ? &a->instructions : nullptr;
    }
};

class AnimationControllerInterface {
    friend class AnimationController;

public:
    AnimationControllerInterface(Controller::BaseController& baseController)
        : _baseController(baseController)
        , _animationController(*this)
        , _instructionController(_animationController)
    {
    }

    auto& baseController() { return _baseController; }

    auto& signal_selectedChanged() { return _signal_selectedChanged; }

    auto& animationController() { return _animationController; }
    auto& instructionController() { return _instructionController; }

protected:
    virtual Animation::map_t* editable_animationMap() = 0;

private:
    Controller::BaseController& _baseController;

    sigc::signal<void> _signal_selectedChanged;

    AnimationController _animationController;
    InstructionController _instructionController;
};

template <class FrameSetControllerT>
class AnimationControllerImpl : public AnimationControllerInterface {

public:
    AnimationControllerImpl(FrameSetControllerT& parent)
        : AnimationControllerInterface(parent.baseController())
        , _parent(parent)
    {
        _parent.signal_selectedChanged().connect(signal_selectedChanged());
    }

    FrameSetControllerT& parent() { return _parent; }

protected:
    virtual Animation::map_t* editable_animationMap() final
    {
        auto* fs = _parent.editable_selected();
        return fs ? &fs->animations : nullptr;
    }

private:
    FrameSetControllerT& _parent;
};

inline Animation::map_t* AnimationController::editable_mapFromParent()
{
    return parent().editable_animationMap();
}
}
}
}
