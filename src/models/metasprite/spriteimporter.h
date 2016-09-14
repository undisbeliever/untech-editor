#pragma once

#include "common.h"
#include "entityhitboxtype.h"
#include "tilesettype.h"
#include "animation/animation.h"
#include "models/common/aabb.h"
#include "models/common/image.h"
#include <map>
#include <string>
#include <vector>

namespace UnTech {
namespace MetaSprite {
namespace SpriteImporter {

const static unsigned MIN_FRAME_SIZE = 16;

struct FrameSet;
struct Frame;

struct FrameSetGrid {
    usize frameSize;
    upoint offset;
    usize padding;
    upoint origin;

    FrameSetGrid()
        : frameSize(MIN_FRAME_SIZE, MIN_FRAME_SIZE)
        , origin(MIN_FRAME_SIZE / 2, MIN_FRAME_SIZE / 2)
    {
    }
    FrameSetGrid(const FrameSetGrid&) = default;

    urect cell(unsigned x, unsigned y) const;
    bool isValid(const FrameSet& frameSet) const;
};

struct FrameLocation {
    urect aabb;
    upoint origin;
    upoint gridLocation;
    bool useGridLocation;
    bool useGridOrigin;

    FrameLocation()
        : aabb(0, 0, MIN_FRAME_SIZE, MIN_FRAME_SIZE)
        , origin(MIN_FRAME_SIZE / 2, MIN_FRAME_SIZE / 2)
    {
    }
    FrameLocation(const FrameLocation&) = default;

    void update(const FrameSetGrid&, const Frame&);
};

struct FrameObject {
    upoint location;
    ObjectSize size;

    FrameObject() = default;
    FrameObject(const upoint& location, ObjectSize& size)
        : location(location)
        , size(size)
    {
    }

    inline unsigned sizePx() const { return static_cast<unsigned>(size); }
    inline upoint bottomLeft() const
    {
        return upoint(location.x + sizePx(),
                      location.y + sizePx());
    }

    bool isValid(const FrameLocation& floc) const
    {
        return floc.aabb.contains(location);
    }
};

struct ActionPoint {
    upoint location;
    ActionPointParameter parameter;

    ActionPoint() = default;
    ActionPoint(const upoint& location, ActionPointParameter& parameter)
        : location(location)
        , parameter(parameter)
    {
    }

    bool isValid(const FrameLocation& floc) const
    {
        return floc.aabb.contains(location);
    }
};

struct EntityHitbox {
    urect aabb;
    EntityHitboxType hitboxType;

    EntityHitbox() = default;
    EntityHitbox(const EntityHitbox&) = default;
    EntityHitbox(const urect& aabb, EntityHitboxType& hitboxType)
        : aabb(aabb)
        , hitboxType(hitboxType)
    {
    }

    bool isValid(const FrameLocation& floc) const
    {
        return floc.aabb.contains(aabb.bottomRight());
    }
};

struct Frame {
    // ::TODO replace with idstring::
    typedef std::map<std::string, Frame> map_t;

    FrameLocation location;
    std::vector<FrameObject> objects;
    std::vector<ActionPoint> actionPoints;
    std::vector<EntityHitbox> entityHitboxes;
    urect tileHitbox;
    SpriteOrderType spriteOrder = DEFAULT_SPRITE_ORDER;
    bool solid;

    Frame() = default;
    Frame(const Frame&) = default;

    usize minimumViableSize() const;
};

struct FrameSet {
    // ::TODO export Order document::

    // ::TODO replace with idstring::
    std::string name;
    TilesetType tilesetType;
    Frame::map_t frames;
    Animation::Animation::map_t animations;

    std::string imageFilename;
    UnTech::Image image;
    UnTech::rgba transparentColor;
    FrameSetGrid grid;

    FrameSet() = default;
    FrameSet(const FrameSet&) = default;

    usize minimumViableFrameSize() const;
    bool transparentColorValid() const
    {
        return transparentColor.alpha == 0xFF;
    }

    void updateFrameLocations();

    void loadImage(const std::string filename);
    bool reloadImage();
};
}
}
}
