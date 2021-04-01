/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "entityhitboxtype.h"
#include "tilesettype.h"
#include "animation/animation.h"
#include "models/common/aabb.h"
#include "models/common/idstring.h"
#include "models/common/image.h"
#include "models/common/namedlist.h"
#include <filesystem>
#include <string>
#include <vector>

namespace UnTech {
class ErrorList;

namespace MetaSprite {
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

    usize originRange() const;

    bool operator==(const FrameSetGrid& o) const;
    bool operator!=(const FrameSetGrid& o) const { return !(*this == o); }

private:
    friend struct FrameSet;
    bool validate(ErrorList& errorList) const;
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

    void update(const FrameSetGrid&, const Frame& frame);

    usize originRange() const;

    bool operator==(const FrameLocation& o) const;
    bool operator!=(const FrameLocation& o) const { return !(*this == o); }

private:
    friend struct Frame;
    bool validate(const Frame& frame, const unsigned frameId, ErrorList& errorList) const;
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

    bool operator==(const FrameObject& o) const
    {
        return this->location == o.location && this->size == o.size;
    }
    bool operator!=(const FrameObject& o) const { return !(*this == o); }
};

struct ActionPoint {
    upoint location;
    idstring type;

    ActionPoint() = default;
    ActionPoint(const upoint& location, idstring& type)
        : location(location)
        , type(type)
    {
    }

    bool operator==(const ActionPoint& o) const
    {
        return this->location == o.location && this->type == o.type;
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

    bool operator==(const EntityHitbox& o) const
    {
        return this->aabb == o.aabb && this->hitboxType == o.hitboxType;
    }
    bool operator!=(const EntityHitbox& o) const { return !(*this == o); }
};

struct Frame {
    idstring name;
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

    usize minimumViableSize() const;

    bool operator==(const Frame& o) const;
    bool operator!=(const Frame& o) const { return !(*this == o); }

private:
    friend struct FrameSet;
    bool validate(const unsigned frameIndex, const Image& image, const ActionPointMapping& actionPointMapping, ErrorList& errorList) const;
};

struct UserSuppliedPalette {
    enum class Position {
        TOP_LEFT,
        TOP_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT,
    };
    static const EnumMap<Position> positionEnumMap;

    Position position;
    unsigned nPalettes;
    unsigned colorSize;

    UserSuppliedPalette()
        : position(Position::TOP_LEFT)
        , nPalettes(0)
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
        return this->position == o.position
               && this->nPalettes == o.nPalettes
               && this->colorSize == o.colorSize;
    }
    bool operator!=(const UserSuppliedPalette& o) const { return !(*this == o); }
};

struct FrameSet {
    static const std::string FILE_EXTENSION;

    idstring name;
    TilesetType tilesetType;
    idstring exportOrder;
    NamedList<Frame> frames;
    NamedList<Animation::Animation> animations;

    std::filesystem::path imageFilename;
    UnTech::rgba transparentColor = rgba(0, 0, 0, 0);
    UserSuppliedPalette palette;
    FrameSetGrid grid;

    FrameSet() = default;

    bool validate(const ActionPointMapping& actionPointMapping, ErrorList& errorList) const;
    bool validate(ErrorList& errorList) const;

    usize minimumFrameGridSize() const;

    void updateFrameLocations();

    bool operator==(const FrameSet& o) const;
    bool operator!=(const FrameSet& o) const { return !(*this == o); }

private:
    bool transparentColorValid(const Image& image) const;
};

std::unique_ptr<FrameSet> loadFrameSet(const std::filesystem::path& filename);
void saveFrameSet(const FrameSet& frameSet, const std::filesystem::path& filename);
}
}
}
