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

public:
    EntityHitbox() = delete;
    EntityHitbox(const EntityHitbox&) = delete;

    EntityHitbox(std::shared_ptr<Frame> frame)
        : _frame(frame)
        , _aabb(-4, -4, 8, 8)
        , _parameter(0)
    {
    }

    EntityHitbox(const EntityHitbox& hitbox, std::shared_ptr<Frame> frame)
        : _frame(frame)
        , _aabb(hitbox._aabb)
        , _parameter(hitbox._parameter)
    {
    }

    std::shared_ptr<EntityHitbox> clone(std::shared_ptr<Frame> frame)
    {
        return std::make_shared<EntityHitbox>(*this, frame);
    }

    inline ms8rect aabb() const { return _aabb; }
    inline parameter_t parameter() const { return _parameter; }

    inline std::shared_ptr<Frame> frame() const { return _frame.lock(); }
    inline MetaSpriteDocument& document() const { return frame()->frameSet()->document(); }

    inline void setAabb(const ms8rect& aabb) { _aabb = aabb; }
    inline void setParameter(parameter_t parameter) { _parameter = parameter; }

private:
    std::weak_ptr<Frame> _frame;
    ms8rect _aabb;
    parameter_t _parameter;
};
}
}

#endif
