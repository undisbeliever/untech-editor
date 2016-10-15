#include "selected.h"

namespace UnTech {
namespace MetaSprite {

template <class BaseControllerT>
SelectedController<BaseControllerT>::SelectedController(BaseControllerT& controller)
    : _controller(controller)
    , _type(SelectedType::NONE)
{
    _controller.frameController().signal_dataChanged().connect(
        [this](void) {
            // deselect TILE_HITBOX is frame is not solid
            if (_type == SelectedType::TILE_HITBOX) {
                auto& f = _controller.frameController().selected();
                if (f.solid == false) {
                    selectFrame();
                }
            }
        });

    _controller.frameController().signal_mapChanged().connect(
        [this](void) {
            if (_type == SelectedType::FRAME || _type == SelectedType::TILE_HITBOX) {
                _signal_listChanged.emit();
            }
        });
    _controller.frameObjectController().signal_listChanged().connect(
        [this](void) {
            if (_type == SelectedType::FRAME_OBJECT) {
                _signal_listChanged.emit();
            }
        });
    _controller.actionPointController().signal_listChanged().connect(
        [this](void) {
            if (_type == SelectedType::ACTION_POINT) {
                _signal_listChanged.emit();
            }
        });
    _controller.entityHitboxController().signal_listChanged().connect(
        [this](void) {
            if (_type == SelectedType::ENTITY_HITBOX) {
                _signal_listChanged.emit();
            }
        });

    _controller.settingsController().layers().signal_layersChanged().connect(sigc::mem_fun(
        *this, &SelectedController::selectNone));

    _controller.frameObjectController().signal_selectedChanged().connect(
        [this](void) {
            if (_controller.frameObjectController().hasSelected()) {
                _type = SelectedType::FRAME_OBJECT;
                _signal_selectedChanged.emit();
            }
            else {
                selectFrame();
            }
        });

    _controller.actionPointController().signal_selectedChanged().connect(
        [this](void) {
            if (_controller.actionPointController().hasSelected()) {
                _type = SelectedType::ACTION_POINT;
                _signal_selectedChanged.emit();
            }
            else {
                selectFrame();
            }
        });

    _controller.entityHitboxController().signal_selectedChanged().connect(
        [this](void) {
            if (_controller.entityHitboxController().hasSelected()) {
                _type = SelectedType::ENTITY_HITBOX;
                _signal_selectedChanged.emit();
            }
            else {
                selectFrame();
            }
        });
}

template <class T>
std::pair<SelectedType, size_t> SelectedController<T>::typeAndIndex() const
{
    size_t id = 0;

    switch (_type) {
    case SelectedType::NONE:
    case SelectedType::FRAME:
    case SelectedType::TILE_HITBOX:
        break;

    case SelectedType::FRAME_OBJECT:
        id = _controller.frameObjectController().selectedIndex().value();
        break;

    case SelectedType::ACTION_POINT:
        id = _controller.actionPointController().selectedIndex().value();
        break;

    case SelectedType::ENTITY_HITBOX:
        id = _controller.entityHitboxController().selectedIndex().value();
        break;
    }

    return { _type, id };
}

template <class T>
void SelectedController<T>::selectNone()
{
    _controller.frameController().selectNone();
}

template <class T>
void SelectedController<T>::selectFrame()
{
    if (_controller.frameController().hasSelected()) {
        if (_type != SelectedType::FRAME) {
            _type = SelectedType::FRAME;
            _signal_selectedChanged.emit();
        }
    }
    else {
        selectNone();
    }
}

template <class T>
void SelectedController<T>::selectTileHitbox()
{
    if (_controller.frameController().selected().solid) {
        if (_type != SelectedType::TILE_HITBOX) {
            _type = SelectedType::TILE_HITBOX;
            _signal_selectedChanged.emit();
        }
    }
    else {
        selectFrame();
    }
}

template <class T>
void SelectedController<T>::selectFrameItem(const idstring& frameId, SelectedType type, size_t index)
{
    if (_controller.frameController().selectedId() != frameId) {
        _controller.frameController().selectId(frameId);
    }

    if (_controller.frameController().hasSelected()) {
        switch (type) {
        case SelectedType::NONE:
        case SelectedType::FRAME:
            selectFrame();
            break;

        case SelectedType::TILE_HITBOX:
            break;

        case SelectedType::FRAME_OBJECT:
            _controller.frameObjectController().selectIndex(index);
            break;

        case SelectedType::ACTION_POINT:
            _controller.actionPointController().selectIndex(index);
            break;

        case SelectedType::ENTITY_HITBOX:
            _controller.entityHitboxController().selectIndex(index);
            break;
        }
    }
}

template <class T>
const std::string& SelectedController<T>::typeString() const
{
    const static std::string nullString;
    const static std::string frameString = "Frame";
    const static std::string tileHitboxString = "Tile Hitbox";
    const static std::string frameObjectString = "Frame Object";
    const static std::string actionPointString = "Action Point";
    const static std::string entityHitboxString = "Entity Hitbox";

    switch (_type) {
    case SelectedType::NONE:
        return nullString;

    case SelectedType::FRAME:
        return frameString;

    case SelectedType::TILE_HITBOX:
        return tileHitboxString;

    case SelectedType::FRAME_OBJECT:
        return frameObjectString;

    case SelectedType::ACTION_POINT:
        return actionPointString;

    case SelectedType::ENTITY_HITBOX:
        return entityHitboxString;
    }

    return nullString;
}

template <class T>
bool SelectedController<T>::canCreateSelected() const
{
    switch (_type) {
    case SelectedType::FRAME_OBJECT:
        return _controller.frameObjectController().canCreate();

    case SelectedType::ACTION_POINT:
        return _controller.actionPointController().canCreate();

    case SelectedType::ENTITY_HITBOX:
        return _controller.entityHitboxController().canCreate();

    default:
        return false;
    }
}

template <class T>
bool SelectedController<T>::canCloneSelected() const
{
    switch (_type) {
    case SelectedType::FRAME_OBJECT:
        return _controller.frameObjectController().canCloneSelected();

    case SelectedType::ACTION_POINT:
        return _controller.actionPointController().canCloneSelected();

    case SelectedType::ENTITY_HITBOX:
        return _controller.entityHitboxController().canCloneSelected();

    default:
        return false;
    }
}

template <class T>
bool SelectedController<T>::canRemoveSelected() const
{
    switch (_type) {
    case SelectedType::FRAME_OBJECT:
        return _controller.frameObjectController().canRemoveSelected();

    case SelectedType::ACTION_POINT:
        return _controller.actionPointController().canRemoveSelected();

    case SelectedType::ENTITY_HITBOX:
        return _controller.entityHitboxController().canRemoveSelected();

    default:
        return false;
    }
}

template <class T>
bool SelectedController<T>::canMoveSelectedUp() const
{
    switch (_type) {
    case SelectedType::FRAME_OBJECT:
        return _controller.frameObjectController().canMoveSelectedUp();

    case SelectedType::ACTION_POINT:
        return _controller.actionPointController().canMoveSelectedUp();

    case SelectedType::ENTITY_HITBOX:
        return _controller.entityHitboxController().canMoveSelectedUp();

    default:
        return false;
    }
}

template <class T>
bool SelectedController<T>::canMoveSelectedDown() const
{
    switch (_type) {
    case SelectedType::FRAME_OBJECT:
        return _controller.frameObjectController().canMoveSelectedDown();

    case SelectedType::ACTION_POINT:
        return _controller.actionPointController().canMoveSelectedDown();

    case SelectedType::ENTITY_HITBOX:
        return _controller.entityHitboxController().canMoveSelectedDown();

    default:
        return false;
    }
}

template <class T>
void SelectedController<T>::createNewOfSelectedType()
{
    switch (_type) {
    case SelectedType::FRAME_OBJECT:
        return _controller.frameObjectController().create();

    case SelectedType::ACTION_POINT:
        return _controller.actionPointController().create();

    case SelectedType::ENTITY_HITBOX:
        return _controller.entityHitboxController().create();

    default:
        return;
    }
}

template <class T>
void SelectedController<T>::cloneSelected()
{
    switch (_type) {
    case SelectedType::FRAME_OBJECT:
        return _controller.frameObjectController().cloneSelected();

    case SelectedType::ACTION_POINT:
        return _controller.actionPointController().cloneSelected();

    case SelectedType::ENTITY_HITBOX:
        return _controller.entityHitboxController().cloneSelected();

    default:
        return;
    }
}

template <class T>
void SelectedController<T>::removeSelected()
{
    switch (_type) {
    case SelectedType::FRAME_OBJECT:
        return _controller.frameObjectController().removeSelected();

    case SelectedType::ACTION_POINT:
        return _controller.actionPointController().removeSelected();

    case SelectedType::ENTITY_HITBOX:
        return _controller.entityHitboxController().removeSelected();

    default:
        return;
    }
}

template <class T>
void SelectedController<T>::moveSelectedUp()
{
    switch (_type) {
    case SelectedType::FRAME_OBJECT:
        return _controller.frameObjectController().moveSelectedUp();

    case SelectedType::ACTION_POINT:
        return _controller.actionPointController().moveSelectedUp();

    case SelectedType::ENTITY_HITBOX:
        return _controller.entityHitboxController().moveSelectedUp();

    default:
        return;
    }
}

template <class T>
void SelectedController<T>::moveSelectedDown()
{
    switch (_type) {
    case SelectedType::FRAME_OBJECT:
        return _controller.frameObjectController().moveSelectedDown();

    case SelectedType::ACTION_POINT:
        return _controller.actionPointController().moveSelectedDown();

    case SelectedType::ENTITY_HITBOX:
        return _controller.entityHitboxController().moveSelectedDown();

    default:
        return;
    }
}
}
}
