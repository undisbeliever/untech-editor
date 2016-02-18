#ifndef _UNTECH_MODELS_SPRITEIMPORTER_ENTITYHITBOX_H
#define _UNTECH_MODELS_SPRITEIMPORTER_ENTITYHITBOX_H

#include "../common/aabb.h"
#include "../common/orderedlist.h"
#include <cstdint>
#include <memory>

namespace UnTech {
namespace SpriteImporter {

class Frame;

class EntityHitbox {

public:
    typedef OrderedList<Frame, EntityHitbox> list_t;

    typedef uint8_t parameter_t;

public:
    EntityHitbox() = delete;
    EntityHitbox(std::shared_ptr<Frame> frame);
    EntityHitbox(const EntityHitbox& hitbox, std::shared_ptr<Frame> frame);

    urect aabb() const { return _aabb; }
    parameter_t parameter() const { return _parameter; }

    std::weak_ptr<Frame> frame() const { return _frame; }

    void setAabb(const urect& aabb);
    void setParameter(parameter_t parameter);

private:
    std::weak_ptr<Frame> _frame;
    urect _aabb;
    parameter_t _parameter;
};
}
}

#endif
