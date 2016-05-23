#pragma once

#include "frame.h"
#include "../common/aabb.h"
#include "../common/orderedlist.h"
#include <cstdint>
#include <memory>

namespace UnTech {
namespace SpriteImporter {

class EntityHitbox {
public:
    typedef OrderedList<Frame, EntityHitbox> list_t;

    typedef uint8_t parameter_t;

public:
    EntityHitbox() = delete;
    EntityHitbox(const EntityHitbox&) = delete;

    EntityHitbox(Frame& frame);
    EntityHitbox(const EntityHitbox& hitbox, Frame& frame);

    inline Frame& frame() const { return _frame; }
    inline SpriteImporterDocument& document() const { return _frame.frameSet().document(); }

    urect aabb() const { return _aabb; }
    parameter_t parameter() const { return _parameter; }

    void setAabb(const urect& aabb);
    void setParameter(parameter_t parameter);

private:
    Frame& _frame;
    urect _aabb;
    parameter_t _parameter;
};
}
}
