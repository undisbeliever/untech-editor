#pragma once

#include "common.h"
#include "entityhitboxtype.h"
#include "frameset-exportorder.h"
#include "tilesettype.h"
#include "animation/animation.h"
#include "models/common/aabb.h"
#include "models/common/capped_vector.h"
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
    typedef capped_vector<FrameObject, MAX_FRAME_OBJECTS> list_t;

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
    typedef capped_vector<ActionPoint, MAX_ACTION_POINTS> list_t;

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
    typedef capped_vector<EntityHitbox, MAX_ENTITY_HITBOXES> list_t;

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
    FrameObject::list_t objects;
    ActionPoint::list_t actionPoints;
    EntityHitbox::list_t entityHitboxes;
    urect tileHitbox;
    SpriteOrderType spriteOrder = DEFAULT_SPRITE_ORDER;
    bool solid;

    Frame() = default;
    Frame(const Frame&) = default;

    usize minimumViableSize() const;
};

struct FrameSet {
    static const std::string FILE_EXTENSION;

    // ::TODO replace with idstring::
    std::string name;
    TilesetType tilesetType;
    std::shared_ptr<const FrameSetExportOrder> exportOrder;
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

std::unique_ptr<FrameSet> loadFrameSet(const std::string& filename);
void saveFrameSet(const FrameSet& frameSet, const std::string& filename);
}
}
}
