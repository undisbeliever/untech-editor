#pragma once
#include "spriteselectedtypecontroller.h"

namespace UnTech {
namespace Controller {

template <class T>
SpriteSelectedTypeController<T>::SpriteSelectedTypeController(T& controller)
    : _controller(controller)
    , _type(Type::NONE)
{
    // SLOTS
    // =====

    /* Controller Signals */
    _controller.frameObjectController().signal_selectedChanged().connect([this](void) {
        if (_controller.frameObjectController().selected() != nullptr) {
            setType(Type::FRAME_OBJECT);
        }
        else if (_type == Type::FRAME_OBJECT) {
            setType(Type::NONE);
        }
        _signal_selectedChanged.emit();
    });
    _controller.frameObjectController().signal_listDataChanged().connect(sigc::hide([this](void) {
        if (_type == Type::FRAME_OBJECT) {
            _signal_listChanged.emit();
        }
    }));

    _controller.actionPointController().signal_selectedChanged().connect([this](void) {
        if (_controller.actionPointController().selected() != nullptr) {
            setType(Type::ACTION_POINT);
        }
        else if (_type == Type::ACTION_POINT) {
            setType(Type::NONE);
        }
        _signal_selectedChanged.emit();
    });
    _controller.actionPointController().signal_listDataChanged().connect(sigc::hide([this](void) {
        if (_type == Type::ACTION_POINT) {
            _signal_listChanged.emit();
        }
    }));

    _controller.entityHitboxController().signal_selectedChanged().connect([this](void) {
        if (_controller.entityHitboxController().selected() != nullptr) {
            setType(Type::ENTITY_HITBOX);
        }
        else if (_type == Type::ENTITY_HITBOX) {
            setType(Type::NONE);
        }
        _signal_selectedChanged.emit();
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
    if (_type != type) {
        _type = type;
        _signal_typeChanged.emit();
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
