#include "selection.h"
#include <glibmm/i18n.h>

using namespace UnTech::Widgets::SpriteImporter;
namespace SI = UnTech::SpriteImporter;

void Selection::setFrameSet(std::shared_ptr<SI::FrameSet> frameSet)
{
    if (_frameSet != frameSet) {
        _frameSet = frameSet;
        signal_frameSetChanged.emit();

        if (_frame != nullptr) {
            _frame = nullptr;
            signal_frameChanged.emit();
        }

        unselectAll();
    }
}

void Selection::setFrame(std::shared_ptr<SI::Frame> frame)
{
    if (_frame != frame) {
        _frame = frame;
        signal_frameChanged.emit();

        unselectAll();
    }
}

void Selection::setFrameObject(std::shared_ptr<SI::FrameObject> frameObject)
{
    bool changed = _frameObject != frameObject;
    if (changed) {
        if (frameObject) {
            auto frame = frameObject->frame();
            if (_frame != frame) {
                auto frameSet = frame->frameSet();
                if (_frameSet != frameSet) {
                    _frameSet = frameSet;
                    signal_frameSetChanged.emit();
                }

                _frame = frame;
                signal_frameChanged.emit();

                if (_actionPoint != nullptr) {
                    _actionPoint = nullptr;
                    signal_actionPointChanged.emit();
                }
                if (_entityHitbox != nullptr) {
                    _entityHitbox = nullptr;
                    signal_entityHitboxChanged.emit();
                }
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

void Selection::setActionPoint(std::shared_ptr<SI::ActionPoint> actionPoint)
{
    bool changed = _actionPoint != actionPoint;
    if (changed) {
        if (actionPoint) {
            auto frame = actionPoint->frame();
            if (_frame != frame) {
                auto frameSet = frame->frameSet();
                if (_frameSet != frameSet) {
                    _frameSet = frameSet;
                    signal_frameSetChanged.emit();
                }

                _frame = frame;
                signal_frameChanged.emit();

                if (_frameObject != nullptr) {
                    _frameObject = nullptr;
                    signal_frameObjectChanged.emit();
                }
                if (_entityHitbox != nullptr) {
                    _entityHitbox = nullptr;
                    signal_entityHitboxChanged.emit();
                }
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

void Selection::setEntityHitbox(std::shared_ptr<SI::EntityHitbox> entityHitbox)
{
    bool changed = _entityHitbox != entityHitbox;
    if (changed) {
        if (entityHitbox) {
            auto frame = entityHitbox->frame();
            if (_frame != frame) {
                auto frameSet = frame->frameSet();
                if (_frameSet != frameSet) {
                    _frameSet = frameSet;
                    signal_frameSetChanged.emit();
                }

                _frame = frame;
                signal_frameChanged.emit();

                if (_frameObject != nullptr) {
                    _frameObject = nullptr;
                    signal_frameObjectChanged.emit();
                }
                if (_entityHitbox != nullptr) {
                    _entityHitbox = nullptr;
                    signal_entityHitboxChanged.emit();
                }
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
