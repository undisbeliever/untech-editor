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
    AnimationController(AnimationControllerInterface& parent)
        : IdMapController(parent)
    {
    }
};

class InstructionController
    : public Controller::CappedVectorController<Instruction, Instruction::list_t,
                                                AnimationController> {

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
    friend class Controller::IdMapController<Animation, AnimationControllerInterface>;

public:
    using element_type = Animation::map_t;

    // ::HACK FrameSet type is unknown::
    // this is allowed as FrameSetController is a root controller
    struct UndoRef {
        enum Type {
            UNKNOWN = 0,
            METASPRITE,
            SPRITE_IMPORTER,
        };
        std::shared_ptr<void> frameSet;
        Type type;
    };

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
    virtual element_type* editable_selected() = 0;

    virtual UndoRef undoRefForSelected() const = 0;
    static element_type& elementFromUndoRef(const UndoRef& ref);

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
    virtual AnimationControllerInterface::element_type* editable_selected() final;
    virtual UndoRef undoRefForSelected() const final;

private:
    FrameSetControllerT& _parent;
};
}
}
}
