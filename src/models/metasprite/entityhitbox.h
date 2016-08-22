#pragma once

#include "frame.h"
#include "models/common/ms8aabb.h"
#include "models/common/orderedlist.h"
#include "models/metasprite-common/entityhitboxtype.h"
#include <cstdint>
#include <memory>

namespace UnTech {
namespace MetaSprite {

class EntityHitbox {
public:
    typedef OrderedList<Frame, EntityHitbox> list_t;
    static const char* TYPE_NAME;

    const static unsigned MAX_HITBOXES = 8;

public:
    EntityHitbox() = delete;
    EntityHitbox(const EntityHitbox&) = delete;

    EntityHitbox(Frame& frame)
        : _frame(frame)
        , _aabb(-4, -4, 8, 8)
        , _hitboxType()
    {
    }

    EntityHitbox(const EntityHitbox& hitbox, Frame& frame)
        : _frame(frame)
        , _aabb(hitbox._aabb)
        , _hitboxType(hitbox._hitboxType)
    {
    }

    inline Frame& frame() const { return _frame; }
    inline MetaSpriteDocument& document() const { return _frame.frameSet().document(); }

    inline ms8rect aabb() const { return _aabb; }
    inline MetaSpriteCommon::EntityHitboxType hitboxType() const { return _hitboxType; }

    inline void setAabb(const ms8rect& aabb)
    {
        _aabb = aabb;
    }

    inline void setHitboxType(MetaSpriteCommon::EntityHitboxType hitboxType)
    {
        _hitboxType = hitboxType;
    }

private:
    Frame& _frame;
    ms8rect _aabb;
    MetaSpriteCommon::EntityHitboxType _hitboxType;
};
}
}
