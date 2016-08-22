#pragma once

#include "frame.h"
#include "models/common/aabb.h"
#include "models/common/orderedlist.h"
#include "models/metasprite-common/entityhitboxtype.h"
#include <cstdint>
#include <memory>

namespace UnTech {
namespace SpriteImporter {

class EntityHitbox {
public:
    typedef OrderedList<Frame, EntityHitbox> list_t;
    static const char* TYPE_NAME;

public:
    EntityHitbox() = delete;
    EntityHitbox(const EntityHitbox&) = delete;

    EntityHitbox(Frame& frame);
    EntityHitbox(const EntityHitbox& hitbox, Frame& frame);

    inline Frame& frame() const { return _frame; }
    inline SpriteImporterDocument& document() const { return _frame.frameSet().document(); }

    urect aabb() const { return _aabb; }
    MetaSpriteCommon::EntityHitboxType hitboxType() const { return _hitboxType; }

    void setAabb(const urect& aabb);
    void setHitboxType(MetaSpriteCommon::EntityHitboxType hitboxType);

private:
    Frame& _frame;
    urect _aabb;
    MetaSpriteCommon::EntityHitboxType _hitboxType;
};
}
}
