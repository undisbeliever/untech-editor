/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "entityhitboxtype.h"
#include "tilesettype.h"
#include "animation/animation.h"
#include "models/common/aabb.h"
#include "models/common/idmap.h"
#include "models/common/idstring.h"
#include "models/common/image.h"
#include <string>
#include <vector>

namespace UnTech {
namespace MetaSprite {
struct ErrorList;

namespace SpriteImporter {

const static unsigned MIN_FRAME_SIZE = 16;
const static unsigned MAX_FRAME_SIZE = 255;
const static unsigned MAX_ORIGIN = 128;

struct FrameSet;
struct Frame;

struct FrameSetGrid {
    usize frameSize;
    upoint offset;
    upoint padding;
    upoint origin;

    FrameSetGrid()
        : frameSize(MIN_FRAME_SIZE, MIN_FRAME_SIZE)
        , offset(0, 0)
        , padding(0, 0)
        , origin(MIN_FRAME_SIZE / 2, MIN_FRAME_SIZE / 2)
    {
    }

    urect cell(unsigned x, unsigned y) const;
    bool isValid(const FrameSet& frameSet) const;

    usize originRange() const;

    bool operator==(const FrameSetGrid& o) const;
    bool operator!=(const FrameSetGrid& o) const { return !(*this == o); }
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
        , gridLocation()
        , useGridLocation(false)
        , useGridOrigin(false)
    {
    }

    void update(const FrameSetGrid&, const Frame&);

    bool isValid(const FrameSet&, const Frame&);

    usize originRange() const;

    bool operator==(const FrameLocation& o) const;
    bool operator!=(const FrameLocation& o) const { return !(*this == o); }
};

struct FrameObject {
    upoint location;
    ObjectSize size;

    FrameObject()
        : location(0, 0)
        , size(ObjectSize::SMALL)
    {
    }

    FrameObject(const upoint& location, ObjectSize& size)
        : location(location)
        , size(size)
    {
    }

    inline unsigned sizePx() const { return static_cast<unsigned>(size); }
    inline upoint bottomRight() const
    {
        return upoint(location.x + sizePx(),
                      location.y + sizePx());
    }

    bool isValid(const FrameLocation& floc) const
    {
        return floc.aabb.size().contains(location, sizePx());
    }

    bool operator==(const FrameObject& o) const
    {
        return this->location == o.location && this->size == o.size;
    }
    bool operator!=(const FrameObject& o) const { return !(*this == o); }
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
        return floc.aabb.size().contains(location);
    }

    bool operator==(const ActionPoint& o) const
    {
        return this->location == o.location && this->parameter == o.parameter;
    }
    bool operator!=(const ActionPoint& o) const { return !(*this == o); }
};

struct EntityHitbox {
    urect aabb;
    EntityHitboxType hitboxType;

    EntityHitbox()
        : aabb(0, 0, MIN_FRAME_SIZE, MIN_FRAME_SIZE)
        , hitboxType()
    {
    }
    EntityHitbox(const urect& aabb, EntityHitboxType& hitboxType)
        : aabb(aabb)
        , hitboxType(hitboxType)
    {
    }

    bool isValid(const FrameLocation& floc) const
    {
        return floc.aabb.size().contains(aabb)
               && aabb.width > 0
               && aabb.height > 0;
    }

    bool operator==(const EntityHitbox& o) const
    {
        return this->aabb == o.aabb && this->hitboxType == o.hitboxType;
    }
    bool operator!=(const EntityHitbox& o) const { return !(*this == o); }
};

struct Frame {
    typedef idmap<Frame> map_t;

    FrameLocation location;
    std::vector<FrameObject> objects;
    std::vector<ActionPoint> actionPoints;
    std::vector<EntityHitbox> entityHitboxes;
    urect tileHitbox;
    SpriteOrderType spriteOrder = DEFAULT_SPRITE_ORDER;
    bool solid;

    Frame()
        : tileHitbox(0, 0, MIN_FRAME_SIZE, MIN_FRAME_SIZE)
        , spriteOrder(DEFAULT_SPRITE_ORDER)
        , solid(false)
    {
    }

    // NOTE: updates the frame location
    bool validate(ErrorList& errorList, const FrameSet& fs);

    usize minimumViableSize() const;

    bool operator==(const Frame& o) const;
    bool operator!=(const Frame& o) const { return !(*this == o); }
};

struct UserSuppliedPalette {
    unsigned nPalettes;
    unsigned colorSize;

    UserSuppliedPalette()
        : nPalettes(0)
        , colorSize(4)
    {
    }

    bool usesUserSuppliedPalette() const
    {
        return nPalettes > 0 && colorSize > 0;
    }

    usize paletteSize() const
    {
        return usize(colorSize * PALETTE_COLORS, nPalettes * colorSize);
    }

    bool operator==(const UserSuppliedPalette& o) const
    {
        return this->nPalettes == o.nPalettes && this->colorSize == o.colorSize;
    }
    bool operator!=(const UserSuppliedPalette& o) const { return !(*this == o); }
};

struct FrameSet {
    static const std::string FILE_EXTENSION;

    idstring name;
    TilesetType tilesetType;
    idstring exportOrder;
    Frame::map_t frames;
    Animation::Animation::map_t animations;

    std::string imageFilename;
    UnTech::rgba transparentColor;
    UserSuppliedPalette palette;
    FrameSetGrid grid;

    FrameSet() = default;

    // NOTE: updates the frame Locations
    bool validate(ErrorList& errorList);

    usize minimumFrameGridSize() const;

    bool transparentColorValid() const
    {
        return transparentColor.alpha == 0xFF;
    }

    void updateFrameLocations();

    bool operator==(const FrameSet& o) const;
    bool operator!=(const FrameSet& o) const { return !(*this == o); }
};

std::unique_ptr<FrameSet> loadFrameSet(const std::string& filename);
void saveFrameSet(const FrameSet& frameSet, const std::string& filename);
}
}
}
