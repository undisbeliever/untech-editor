#pragma once

#include "common.h"
#include "entityhitboxtype.h"
#include "frameset-exportorder.h"
#include "tilesettype.h"
#include "animation/animation.h"
#include "models/common/capped_vector.h"
#include "models/common/idmap.h"
#include "models/common/idstring.h"
#include "models/common/image.h"
#include "models/common/ms8aabb.h"
#include "models/snes/tileset.h"
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
    EntityHitbox(const ms8rect& aabb, EntityHitboxType& hitboxType)
        : aabb(aabb)
        , hitboxType(hitboxType)
    {
    }
};

struct Frame {
    typedef idmap<Frame> map_t;

    FrameObject::list_t objects;
    ActionPoint::list_t actionPoints;
    EntityHitbox::list_t entityHitboxes;
    ms8rect tileHitbox;
    bool solid;

    Frame() = default;

    Frame flip(bool hFlip, bool vFlip) const;

    void draw(Image& image, const FrameSet& frameSet, size_t paletteId,
              unsigned xOffset, unsigned yOffset) const;
};

struct FrameSet {
    static const std::string FILE_EXTENSION;

    idstring name;
    TilesetType tilesetType;
    std::shared_ptr<const FrameSetExportOrder> exportOrder;
    Frame::map_t frames;
    Animation::Animation::map_t animations;

    Snes::Tileset4bpp8px smallTileset;
    Snes::Tileset4bpp16px largeTileset;
    capped_vector<Snes::Palette4bpp, MAX_PALETTES> palettes;

    FrameSet() = default;

    // ::TODO isValid - returns a string if there is an error::
};

std::unique_ptr<FrameSet> loadFrameSet(const std::string& filename);
void saveFrameSet(const FrameSet& frameSet, const std::string& filename);
}
}
}
