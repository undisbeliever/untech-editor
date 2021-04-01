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
#include "models/common/idstring.h"
#include "models/common/ms8aabb.h"
#include "models/common/namedlist.h"
#include "models/snes/snescolor.h"
#include "models/snes/tile.h"
#include <filesystem>
#include <string>

namespace UnTech {
class ErrorList;

namespace MetaSprite {
namespace MetaSprite {

struct FrameSet;
struct Frame;

struct FrameObject {
    ms8point location;
    ObjectSize size;
    unsigned tileId;
    bool hFlip;
    bool vFlip;

    FrameObject()
        : location(-4, -4)
        , size(ObjectSize::SMALL)
        , tileId(0)
        , hFlip(false)
        , vFlip(false)
    {
    }
    FrameObject(const ms8point& location, ObjectSize& size)
        : location(location)
        , size(size)
        , tileId(0)
        , hFlip(false)
        , vFlip(false)
    {
    }

    inline unsigned sizePx() const { return static_cast<unsigned>(size); }

    bool operator==(const FrameObject& o) const
    {
        return this->location == o.location && this->size == o.size
               && this->tileId == o.tileId && this->hFlip == o.hFlip
               && this->vFlip == o.vFlip;
    }
    bool operator!=(const FrameObject& o) const { return !(*this == o); }
};

struct ActionPoint {
    ms8point location;
    idstring type;

    ActionPoint() = default;
    ActionPoint(const ms8point& location, const idstring& type)
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
    ms8rect aabb;
    EntityHitboxType hitboxType;

    EntityHitbox()
        : aabb(-8, -8, 16, 16)
        , hitboxType()
    {
    }
    EntityHitbox(const ms8rect& aabb, EntityHitboxType& hitboxType)
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
    std::vector<FrameObject> objects;
    std::vector<ActionPoint> actionPoints;
    std::vector<EntityHitbox> entityHitboxes;
    SpriteOrderType spriteOrder = DEFAULT_SPRITE_ORDER;
    ms8rect tileHitbox;
    bool solid;

    Frame()
        : spriteOrder(DEFAULT_SPRITE_ORDER)
        , tileHitbox(-8, -8, 16, 16)
        , solid(false)
    {
    }

    Frame flip(bool hFlip, bool vFlip) const;

    bool operator==(const Frame& o) const;
    bool operator!=(const Frame& o) const { return !(*this == o); }

private:
    friend struct FrameSet;
    bool validate(const unsigned frameIndex, const FrameSet& fs,
                  const ActionPointMapping& actionPointMapping, ErrorList& errorList) const;
};

struct FrameSet {
    static const std::string FILE_EXTENSION;

    idstring name;
    TilesetType tilesetType;
    idstring exportOrder;
    NamedList<Frame> frames;
    NamedList<Animation::Animation> animations;

    std::vector<Snes::Tile8px> smallTileset;
    std::vector<Snes::Tile16px> largeTileset;
    std::vector<Snes::Palette4bpp> palettes;

    FrameSet()
        : name()
        , tilesetType(TilesetType::Enum::ONE_ROW)
        , exportOrder()
        , frames()
        , animations()
        , smallTileset()
        , largeTileset()
        , palettes()
    {
    }

    bool validate(const ActionPointMapping& actionPointMapping, ErrorList& errorList) const;

    bool operator==(const FrameSet& o) const;
    bool operator!=(const FrameSet& o) const { return !(*this == o); }
};

std::unique_ptr<FrameSet> loadFrameSet(const std::filesystem::path& filename);
void saveFrameSet(const FrameSet& frameSet, const std::filesystem::path& filename);
}
}
}
