#include "selection.h"
#include "signals.h"
#include "gui/undo/orderedlistactions.h"
#include <glibmm/i18n.h>

using namespace UnTech::Widgets::MetaSprite;
namespace MS = UnTech::MetaSprite;

void Selection::updateFrameSet(std::shared_ptr<MS::FrameSet> frameSet)
{
    if (_frameSet != frameSet) {
        _frameSet = frameSet;
        signal_frameSetChanged.emit();

        setPalette(nullptr);
    }
}

void Selection::setFrameSet(std::shared_ptr<MS::FrameSet> frameSet)
{
    updateFrameSet(frameSet);

    if (_frame != nullptr) {
        _frame = nullptr;
        signal_frameChanged.emit();
    }

    unselectAll();
}

void Selection::setPalette(std::shared_ptr<MS::Palette> palette)
{
    if (palette == nullptr && _frameSet && _frameSet->palettes().size() > 0) {
        // auto select first palette
        return setPalette(*(_frameSet->palettes().begin()));
    }

    if (_palette != palette) {
        _palette = palette;
        signal_paletteChanged.emit();
    }
}

void Selection::setFrame(std::shared_ptr<MS::Frame> frame)
{
    if (_frame != frame) {
        updateFrame(frame);

        unselectAll();
    }
}

void Selection::updateFrame(std::shared_ptr<MS::Frame> frame)
{
    if (_frame != frame) {
        _frame = frame;
        signal_frameChanged.emit();

        if (frame != nullptr) {
            updateFrameSet(frame->frameSet());
        }
    }
}

void Selection::setFrameObject(std::shared_ptr<MS::FrameObject> frameObject)
{
    bool changed = _frameObject != frameObject;
    if (changed) {
        if (frameObject) {
            updateFrame(frameObject->frame());

            if (_actionPoint != nullptr) {
                _actionPoint = nullptr;
                signal_actionPointChanged.emit();
            }
            if (_entityHitbox != nullptr) {
                _entityHitbox = nullptr;
                signal_entityHitboxChanged.emit();
            }
        }

        if (_frameObject != frameObject) {
            _frameObject = frameObject;
            signal_frameObjectChanged.emit();
        }
    }

    Type oldType = _type;
    _type = frameObject != nullptr ? Type::FRAME_OBJECT : Type::NONE;

    if (changed || oldType != _type) {
        signal_selectionChanged.emit();
    }
}

void Selection::setActionPoint(std::shared_ptr<MS::ActionPoint> actionPoint)
{
    bool changed = _actionPoint != actionPoint;
    if (changed) {
        if (actionPoint) {
            updateFrame(actionPoint->frame());

            if (_frameObject != nullptr) {
                _frameObject = nullptr;
                signal_frameObjectChanged.emit();
            }
            if (_entityHitbox != nullptr) {
                _entityHitbox = nullptr;
                signal_entityHitboxChanged.emit();
            }
        }

        if (_actionPoint != actionPoint) {
            _actionPoint = actionPoint;
            signal_actionPointChanged();
        }
    }

    Type oldType = _type;
    _type = actionPoint != nullptr ? Type::ACTION_POINT : Type::NONE;

    if (changed || oldType != _type) {
        signal_selectionChanged.emit();
    }
}

void Selection::setEntityHitbox(std::shared_ptr<MS::EntityHitbox> entityHitbox)
{
    bool changed = _entityHitbox != entityHitbox;
    if (changed) {
        if (entityHitbox) {
            updateFrame(entityHitbox->frame());

            if (_frameObject != nullptr) {
                _frameObject = nullptr;
                signal_frameObjectChanged.emit();
            }
            if (_entityHitbox != nullptr) {
                _entityHitbox = nullptr;
                signal_entityHitboxChanged.emit();
            }
        }

        if (_entityHitbox != entityHitbox) {
            _entityHitbox = entityHitbox;
            signal_entityHitboxChanged();
        }
    }

    Type oldType = _type;
    _type = entityHitbox != nullptr ? Type::ENTITY_HITBOX : Type::NONE;

    if (changed || oldType != _type) {
        signal_selectionChanged.emit();
    }
}

void Selection::unselectAll()
{
    if (_frameObject != nullptr) {
        _frameObject = nullptr;
        signal_frameObjectChanged.emit();
    }
    if (_actionPoint != nullptr) {
        _actionPoint = nullptr;
        signal_actionPointChanged();
    }
    if (_entityHitbox != nullptr) {
        _entityHitbox = nullptr;
        signal_entityHitboxChanged();
    }

    _type = Selection::Type::NONE;

    signal_selectionChanged.emit();
}

Glib::ustring Selection::typeString() const
{
    switch (_type) {
    case Type::FRAME_OBJECT:
        return _("Frame Object");

    case Type::ACTION_POINT:
        return _("Action Point");

    case Type::ENTITY_HITBOX:
        return _("Entity Hitbox");

    default:
        return "";
    }
}

bool Selection::canMoveSelectedUp() const
{
    switch (_type) {
    case Type::FRAME_OBJECT:
        return !_frame->objects().isFirst(_frameObject);

    case Type::ACTION_POINT:
        return !_frame->actionPoints().isFirst(_actionPoint);

    case Type::ENTITY_HITBOX:
        return !_frame->entityHitboxes().isFirst(_entityHitbox);

    default:
        return false;
    }
}

bool Selection::canMoveSelectedDown() const
{
    switch (_type) {
    case Type::FRAME_OBJECT:
        return !_frame->objects().isLast(_frameObject);

    case Type::ACTION_POINT:
        return !_frame->actionPoints().isLast(_actionPoint);

    case Type::ENTITY_HITBOX:
        return !_frame->entityHitboxes().isLast(_entityHitbox);

    default:
        return false;
    }
}

void Selection::createNewOfSelectedType()
{
    switch (_type) {
    case Type::FRAME_OBJECT:
        setFrameObject(
            Undo::orderedList_create<MS::FrameObject>(
                &_frame->objects(),
                Signals::frameObjectListChanged,
                _("Create new Frame Object")));
        break;

    case Type::ACTION_POINT:
        setActionPoint(
            Undo::orderedList_create<MS::ActionPoint>(
                &_frame->actionPoints(),
                Signals::actionPointListChanged,
                _("Create new Action Point")));
        break;

    case Type::ENTITY_HITBOX:
        setEntityHitbox(
            Undo::orderedList_create<MS::EntityHitbox>(
                &_frame->entityHitboxes(),
                Signals::entityHitboxListChanged,
                _("Create new Entity Hitbox")));
        break;

    default:
        break;
    }
}

void Selection::cloneSelected()
{
    switch (_type) {
    case Type::FRAME_OBJECT:
        setFrameObject(
            Undo::orderedList_clone<MS::FrameObject>(
                &_frame->objects(), _frameObject,
                Signals::frameObjectListChanged,
                _("Clone Frame Object")));
        break;

    case Type::ACTION_POINT:
        setActionPoint(
            Undo::orderedList_clone<MS::ActionPoint>(
                &_frame->actionPoints(), _actionPoint,
                Signals::actionPointListChanged,
                _("Clone Action Point")));
        break;

    case Type::ENTITY_HITBOX:
        setEntityHitbox(
            Undo::orderedList_clone<MS::EntityHitbox>(
                &_frame->entityHitboxes(), _entityHitbox,
                Signals::entityHitboxListChanged,
                _("Clone Entity Hitbox")));
        break;

    default:
        break;
    }
}

void Selection::removeSelected()
{
    switch (_type) {
    case Type::FRAME_OBJECT:
        Undo::orderedList_remove<MS::FrameObject>(
            &_frame->objects(), _frameObject,
            Signals::frameObjectListChanged,
            _("Remove Frame Object"));
        setFrameObject(nullptr);
        break;

    case Type::ACTION_POINT:
        Undo::orderedList_remove<MS::ActionPoint>(
            &_frame->actionPoints(), _actionPoint,
            Signals::actionPointListChanged,
            _("Remove Action Point"));
        setActionPoint(nullptr);
        break;

    case Type::ENTITY_HITBOX:
        Undo::orderedList_remove<MS::EntityHitbox>(
            &_frame->entityHitboxes(), _entityHitbox,
            Signals::entityHitboxListChanged,
            _("Remove Entity Hitbox"));
        setEntityHitbox(nullptr);
        break;

    default:
        break;
    }
}

void Selection::moveSelectedUp()
{
    switch (_type) {
    case Type::FRAME_OBJECT:
        Undo::orderedList_moveUp<MS::FrameObject>(
            &_frame->objects(), _frameObject,
            Signals::frameObjectListChanged,
            _("Move Frame Object up"));
        break;

    case Type::ACTION_POINT:
        Undo::orderedList_moveUp<MS::ActionPoint>(
            &_frame->actionPoints(), _actionPoint,
            Signals::actionPointListChanged,
            _("Move Action Point up"));
        break;

    case Type::ENTITY_HITBOX:
        Undo::orderedList_moveUp<MS::EntityHitbox>(
            &_frame->entityHitboxes(), _entityHitbox,
            Signals::entityHitboxListChanged,
            _("Move Entity Hitbox up"));
        break;

    default:
        break;
    }
}

void Selection::moveSelectedDown()
{
    switch (_type) {
    case Type::FRAME_OBJECT:
        Undo::orderedList_moveDown<MS::FrameObject>(
            &_frame->objects(), _frameObject,
            Signals::frameObjectListChanged,
            _("Move Frame Object down"));
        break;

    case Type::ACTION_POINT:
        Undo::orderedList_moveDown<MS::ActionPoint>(
            &_frame->actionPoints(), _actionPoint,
            Signals::actionPointListChanged,
            _("Move Action Point down"));
        break;

    case Type::ENTITY_HITBOX:
        Undo::orderedList_moveDown<MS::EntityHitbox>(
            &_frame->entityHitboxes(), _entityHitbox,
            Signals::entityHitboxListChanged,
            _("Move Entity Hitbox down"));
        break;

    default:
        break;
    }
}
