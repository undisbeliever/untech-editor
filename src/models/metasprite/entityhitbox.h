#ifndef _UNTECH_MODELS_METASPRITE_ENTITYHITBOX_H
#define _UNTECH_MODELS_METASPRITE_ENTITYHITBOX_H

#include "frame.h"
#include "../common/ms8aabb.h"
#include "../common/orderedlist.h"
#include <cstdint>
#include <memory>

namespace UnTech {
namespace MetaSprite {

class EntityHitbox {
public:
    typedef OrderedList<Frame, EntityHitbox> list_t;

    typedef uint8_t parameter_t;

    const static unsigned MAX_HITBOXES = 8;

public:
    EntityHitbox() = delete;
    EntityHitbox(const EntityHitbox&) = delete;

    EntityHitbox(Frame& frame)
        : _frame(frame)
        , _aabb(-4, -4, 8, 8)
        , _parameter(0)
    {
    }

    EntityHitbox(const EntityHitbox& hitbox, Frame& frame)
        : _frame(frame)
        , _aabb(hitbox._aabb)
        , _parameter(hitbox._parameter)
    {
    }

    inline Frame& frame() const { return _frame; }
    inline MetaSpriteDocument& document() const { return _frame.frameSet().document(); }

    inline ms8rect aabb() const { return _aabb; }
    inline parameter_t parameter() const { return _parameter; }

    inline void setAabb(const ms8rect& aabb) { _aabb = aabb; }
    inline void setParameter(parameter_t parameter) { _parameter = parameter; }

private:
    Frame& _frame;
    ms8rect _aabb;
    parameter_t _parameter;
};
}
}

#endif
