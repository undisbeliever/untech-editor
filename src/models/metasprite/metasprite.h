/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "entityhitboxtype.h"
#include "frameset-exportorder.h"
#include "tilesettype.h"
#include "animation/animation.h"
#include "models/common/idmap.h"
#include "models/common/idstring.h"
#include "models/common/image.h"
#include "models/common/ms8aabb.h"
#include "models/snes/tileset.h"
#include <string>

namespace UnTech {
namespace MetaSprite {
struct ErrorList;

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

    bool isValid(const FrameSet&) const;

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
    ActionPointParameter parameter;

    ActionPoint() = default;
    ActionPoint(const ms8point& location, ActionPointParameter& parameter)
        : location(location)
        , parameter(parameter)
    {
    }

    bool operator==(const ActionPoint& o) const
    {
        return this->location == o.location && this->parameter == o.parameter;
    }
    bool operator!=(const ActionPoint& o) const { return !(*this == o); }
};

struct EntityHitbox {
    ms8rect aabb;
    EntityHitboxType hitboxType;

    EntityHitbox()
        : aabb(-8, -8, 16, 16)
        , hitboxType(EntityHitboxType::Enum::BODY)
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
    typedef idmap<Frame> map_t;

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

    bool validate(ErrorList& errorList, const FrameSet& fs) const;

    Frame flip(bool hFlip, bool vFlip) const;

    void draw(Image& image, const FrameSet& frameSet, size_t paletteId,
              unsigned xOffset, unsigned yOffset) const;

    bool operator==(const Frame& o) const;
    bool operator!=(const Frame& o) const { return !(*this == o); }
};

struct FrameSet {
    static const std::string FILE_EXTENSION;

    idstring name;
    TilesetType tilesetType;
    idstring exportOrder;
    Frame::map_t frames;
    Animation::Animation::map_t animations;

    Snes::Tileset8px smallTileset;
    Snes::TilesetTile16 largeTileset;
    std::vector<Snes::Palette4bpp> palettes;

    FrameSet()
        : name()
        , tilesetType(TilesetType::Enum::ONE_ROW)
        , exportOrder()
        , frames()
        , animations()
        , smallTileset(Snes::Tileset8px::BitDepth::BD_4BPP)
        , largeTileset()
        , palettes()
    {
    }

    bool validate(ErrorList& errorList) const;

    bool operator==(const FrameSet& o) const;
    bool operator!=(const FrameSet& o) const { return !(*this == o); }
};

std::unique_ptr<FrameSet> loadFrameSet(const std::string& filename);
void saveFrameSet(const FrameSet& frameSet, const std::string& filename);
}
}
}
