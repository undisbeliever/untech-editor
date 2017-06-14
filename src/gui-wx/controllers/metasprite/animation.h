/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../containers/cappedvectorcontroller.h"
#include "../containers/idmapcontroller.h"
#include "models/metasprite/animation/animation.h"

namespace UnTech {
namespace MetaSprite {
namespace Animation {

class AnimationControllerInterface;
class AnimationController;
class AnimationFrameController;

class AnimationController
    : public Controller::IdMapController<Animation, AnimationControllerInterface> {

    friend class AnimationFrameController;

public:
    AnimationController(AnimationControllerInterface& parent)
        : IdMapController(parent)
    {
    }

    void selected_setDurationFormat(DurationFormat format);
    void selected_setNextAnimation(const idstring& animationId);
    void selected_setOneShot(bool oneShot);
};

class AnimationFrameController
    : public Controller::CappedVectorController<AnimationFrame, AnimationFrame::list_t,
                                                AnimationController> {

public:
    AnimationFrameController(AnimationController& parent)
        : CappedVectorController(parent)
    {
    }

    void selected_setFrame(const NameReference& frame);
    void selected_setDuration(uint8_t duration);

protected:
    virtual AnimationFrame::list_t* editable_listFromParent() final
    {
        Animation* a = parent().editable_selected();
        return a ? &a->frames : nullptr;
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

        bool operator==(const UndoRef& o) const { return frameSet == o.frameSet && type == o.type; }
    };

public:
    AnimationControllerInterface(Controller::BaseController& baseController)
        : _baseController(baseController)
        , _animationController(*this)
        , _animationFrameController(_animationController)
    {
    }

    auto& baseController() { return _baseController; }

    auto& signal_selectedChanged() { return _signal_selectedChanged; }

    auto& animationController() { return _animationController; }
    auto& animationFrameController() { return _animationFrameController; }

protected:
    virtual element_type* editable_selected() = 0;

    virtual UndoRef undoRefForSelected() const = 0;
    static element_type& elementFromUndoRef(const UndoRef& ref);

private:
    Controller::BaseController& _baseController;

    sigc::signal<void> _signal_selectedChanged;

    AnimationController _animationController;
    AnimationFrameController _animationFrameController;
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
