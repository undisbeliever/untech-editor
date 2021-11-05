/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "entityhitboxtype.h"
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
}

namespace UnTech::MetaSprite::MetaSprite {

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

    inline unsigned sizePx() const { return size == ObjectSize::SMALL ? 8 : 16; }

    bool operator==(const FrameObject&) const = default;
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

    bool operator==(const ActionPoint&) const = default;
};

struct CollisionBox {
    ms8rect aabb;
    bool exists;

    CollisionBox()
        : aabb(-8, -8, 16, 16)
        , exists(false)
    {
    }

    bool operator==(const CollisionBox&) const = default;
};

struct Frame {
    idstring name;
    std::vector<FrameObject> objects;
    std::vector<ActionPoint> actionPoints;
    SpriteOrderType spriteOrder = DEFAULT_SPRITE_ORDER;

    CollisionBox tileHitbox;
    CollisionBox shield;
    CollisionBox hitbox;
    CollisionBox hurtbox;

    Frame()
        : spriteOrder(DEFAULT_SPRITE_ORDER)
    {
    }

    Frame flip(bool hFlip, bool vFlip) const;

    bool operator==(const Frame&) const = default;
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
        , tilesetType(TilesetType::ONE_ROW)
        , exportOrder()
        , frames()
        , animations()
        , smallTileset()
        , largeTileset()
        , palettes()
    {
    }

    bool operator==(const FrameSet&) const = default;
};

bool validate(const FrameSet& input, const ActionPointMapping& actionPointMapping, ErrorList& errorList);

std::unique_ptr<FrameSet> loadFrameSet(const std::filesystem::path& filename);
void saveFrameSet(const FrameSet& frameSet, const std::filesystem::path& filename);

}
