#include "entityhitbox.h"

using namespace UnTech;
using namespace SpriteImporter;

const char* EntityHitbox::TYPE_NAME = "Entity Hitbox";

EntityHitbox::EntityHitbox(Frame& frame)
    : _frame(frame)
    , _aabb(0, 0, Frame::MIN_WIDTH, Frame::MIN_HEIGHT)
    , _hitboxType()
{
    const auto frameLocation = frame.location();

    _aabb.x = (frameLocation.width - Frame::MIN_WIDTH) / 2;
    _aabb.y = (frameLocation.height - Frame::MIN_HEIGHT) / 2;
}

EntityHitbox::EntityHitbox(const EntityHitbox& hitbox, Frame& frame)
    : _frame(frame)
    , _aabb(hitbox._aabb)
    , _hitboxType(hitbox._hitboxType)
{
}

void EntityHitbox::setAabb(const urect& aabb)
{
    urect newAabb = _frame.location().clipInside(aabb, _aabb);

    if (_aabb != newAabb) {
        _aabb = newAabb;
    }
}

void EntityHitbox::setHitboxType(MetaSpriteCommon::EntityHitboxType hitboxType)
{
    if (_hitboxType != hitboxType) {
        _hitboxType = hitboxType;
    }
}
