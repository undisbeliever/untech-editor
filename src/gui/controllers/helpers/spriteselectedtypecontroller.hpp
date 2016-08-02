#pragma once
#include "spriteselectedtypecontroller.h"

namespace UnTech {
namespace Controller {

template <class T>
SpriteSelectedTypeController<T>::SpriteSelectedTypeController(T& controller)
    : _controller(controller)
    , _type(Type::NONE)
    , _updatingType(false)
{
    // SLOTS
    // =====

    /* Controller Signals */
    _controller.layersController().signal_layersChanged().connect([this](void) {
        // prevent selected from being shown on a disabled layer
        if (_type != Type::NONE) {
            setType(Type::NONE);
        }
    });

    _controller.frameObjectController().signal_selectedChanged().connect([this](void) {
        if (_controller.layersController().frameObjects()) {
            if (_controller.frameObjectController().selected() != nullptr) {
                setType(Type::FRAME_OBJECT);
            }
            else {
                setType(Type::NONE);
            }
        }
    });
    _controller.frameObjectController().signal_listDataChanged().connect(sigc::hide([this](void) {
        if (_type == Type::FRAME_OBJECT) {
            _signal_listChanged.emit();
        }
    }));

    _controller.actionPointController().signal_selectedChanged().connect([this](void) {
        if (_controller.layersController().actionPoints()) {
            if (_controller.actionPointController().selected() != nullptr) {
                setType(Type::ACTION_POINT);
            }
            else {
                setType(Type::NONE);
            }
        }
    });
    _controller.actionPointController().signal_listDataChanged().connect(sigc::hide([this](void) {
        if (_type == Type::ACTION_POINT) {
            _signal_listChanged.emit();
        }
    }));

    _controller.entityHitboxController().signal_selectedChanged().connect([this](void) {
        if (_controller.layersController().entityHitboxes()) {
            if (_controller.entityHitboxController().selected() != nullptr) {
                setType(Type::ENTITY_HITBOX);
            }
            else {
                setType(Type::NONE);
            }
        }
    });
    _controller.entityHitboxController().signal_listDataChanged().connect(sigc::hide([this](void) {
        if (_type == Type::ENTITY_HITBOX) {
            _signal_listChanged.emit();
        }
    }));
}

template <class T>
inline void SpriteSelectedTypeController<T>::setType(Type type)
{
    /*
     * Unselect the items not of Type.
     *
     * Because of the way signals propagate we only want the
     * first SetType call to be reconsigned.
     */
    if (_updatingType == false) {
        if (_type != type) {
            _updatingType = true;

            _type = type;
            _signal_typeChanged.emit();

            if (type != Type::FRAME_OBJECT) {
                _controller.frameObjectController().setSelected(nullptr);
            }
            if (type != Type::ACTION_POINT) {
                _controller.actionPointController().setSelected(nullptr);
            }
            if (type != Type::ENTITY_HITBOX) {
                _controller.entityHitboxController().setSelected(nullptr);
            }

            _updatingType = false;
        }

        _signal_selectedChanged.emit();
    }
}

template <class T>
const std::string& SpriteSelectedTypeController<T>::typeString() const
{
    const static std::string nullString;
    const static std::string frameObjectString = "Frame Object";
    const static std::string actionPointString = "Action Point";
    const static std::string entityHitboxString = "Entity Hitbox";

    switch (_type) {
    case Type::FRAME_OBJECT:
        return frameObjectString;

    case Type::ACTION_POINT:
        return actionPointString;

    case Type::ENTITY_HITBOX:
        return entityHitboxString;

    default:
        return nullString;
    }
}

template <class T>
bool SpriteSelectedTypeController<T>::canMoveSelectedUp() const
{
    switch (_type) {
    case Type::FRAME_OBJECT:
        return _controller.frameObjectController().canMoveSelectedUp();

    case Type::ACTION_POINT:
        return _controller.actionPointController().canMoveSelectedUp();

    case Type::ENTITY_HITBOX:
        return _controller.entityHitboxController().canMoveSelectedUp();

    default:
        return false;
    }
}

template <class T>
bool SpriteSelectedTypeController<T>::canMoveSelectedDown() const
{
    switch (_type) {
    case Type::FRAME_OBJECT:
        return _controller.frameObjectController().canMoveSelectedDown();

    case Type::ACTION_POINT:
        return _controller.actionPointController().canMoveSelectedDown();

    case Type::ENTITY_HITBOX:
        return _controller.entityHitboxController().canMoveSelectedDown();

    default:
        return false;
    }
}

template <class T>
void SpriteSelectedTypeController<T>::createNewOfSelectedType()
{
    switch (_type) {
    case Type::FRAME_OBJECT:
        return _controller.frameObjectController().create();

    case Type::ACTION_POINT:
        return _controller.actionPointController().create();

    case Type::ENTITY_HITBOX:
        return _controller.entityHitboxController().create();

    default:
        return;
    }
}

template <class T>
void SpriteSelectedTypeController<T>::cloneSelected()
{
    switch (_type) {
    case Type::FRAME_OBJECT:
        return _controller.frameObjectController().selected_clone();

    case Type::ACTION_POINT:
        return _controller.actionPointController().selected_clone();

    case Type::ENTITY_HITBOX:
        return _controller.entityHitboxController().selected_clone();

    default:
        return;
    }
}

template <class T>
void SpriteSelectedTypeController<T>::removeSelected()
{
    switch (_type) {
    case Type::FRAME_OBJECT:
        return _controller.frameObjectController().selected_remove();

    case Type::ACTION_POINT:
        return _controller.actionPointController().selected_remove();

    case Type::ENTITY_HITBOX:
        return _controller.entityHitboxController().selected_remove();

    default:
        return;
    }
}

template <class T>
void SpriteSelectedTypeController<T>::moveSelectedUp()
{
    switch (_type) {
    case Type::FRAME_OBJECT:
        return _controller.frameObjectController().selected_moveUp();

    case Type::ACTION_POINT:
        return _controller.actionPointController().selected_moveUp();

    case Type::ENTITY_HITBOX:
        return _controller.entityHitboxController().selected_moveUp();

    default:
        return;
    }
}

template <class T>
void SpriteSelectedTypeController<T>::moveSelectedDown()
{
    switch (_type) {
    case Type::FRAME_OBJECT:
        return _controller.frameObjectController().selected_moveDown();

    case Type::ACTION_POINT:
        return _controller.actionPointController().selected_moveDown();

    case Type::ENTITY_HITBOX:
        return _controller.entityHitboxController().selected_moveDown();

    default:
        return;
    }
}
}
}
