#pragma once

#include "common.h"
#include "entityhitboxtype.h"
#include "tilesettype.h"
#include "animation/animation.h"
#include "models/common/capped_vector.h"
#include "models/common/image.h"
#include "models/common/ms8aabb.h"
#include "models/snes/tileset.h"
#include <map>
#include <string>

namespace UnTech {
namespace MetaSprite {
namespace MetaSprite {

struct FrameSet;
struct Frame;

struct FrameObject {
    typedef capped_vector<FrameObject, MAX_FRAME_OBJECTS> list_t;

    ms8point location;
    ObjectSize size;
    unsigned tileId;
    SpriteOrderType order = DEFAULT_SPRITE_ORDER;
    bool hFlip;
    bool vFlip;

    FrameObject() = default;
    FrameObject(const ms8point& location, ObjectSize& size)
        : location(location)
        , size(size)
    {
    }

    inline unsigned sizePx() const { return static_cast<unsigned>(size); }

    bool isValid(const FrameSet&);
};

struct ActionPoint {
    typedef capped_vector<ActionPoint, MAX_ACTION_POINTS> list_t;

    ms8point location;
    ActionPointParameter parameter;

    ActionPoint() = default;
    ActionPoint(const ms8point& location, ActionPointParameter& parameter)
        : location(location)
        , parameter(parameter)
    {
    }
};

struct EntityHitbox {
    typedef capped_vector<EntityHitbox, MAX_ENTITY_HITBOXES> list_t;

    ms8rect aabb;
    EntityHitboxType hitboxType;

    EntityHitbox() = default;
    EntityHitbox(const EntityHitbox&) = default;
    EntityHitbox(const ms8rect& aabb, EntityHitboxType& hitboxType)
        : aabb(aabb)
        , hitboxType(hitboxType)
    {
    }
};

struct Frame {
    // ::TODO replace with idstring::
    typedef std::map<std::string, Frame> map_t;

    FrameObject::list_t objects;
    ActionPoint::list_t actionPoints;
    EntityHitbox::list_t entityHitboxes;
    ms8rect tileHitbox;
    bool solid;

    Frame() = default;
    Frame(const Frame&) = default;
    Frame(Frame&&) = default;

    Frame flip(bool hFlip, bool vFlip) const;

    void draw(Image& image, const FrameSet& frameSet, size_t paletteId,
              unsigned xOffset, unsigned yOffset) const;
};

struct FrameSet {
    // ::TODO export Order document::

    // ::TODO replace with idstring::
    std::string name;
    TilesetType tilesetType;
    Frame::map_t frames;
    Animation::Animation::map_t animations;

    Snes::Tileset4bpp8px smallTileset;
    Snes::Tileset4bpp16px largeTileset;
    std::vector<Snes::Palette4bpp> palettes;

    FrameSet() = default;
    FrameSet(const FrameSet&) = default;

    // ::TODO isValid - returns a string if there is an error::
};
}
}
}
