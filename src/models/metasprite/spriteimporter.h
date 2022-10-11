/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "animation/animation.h"
#include "models/common/aabb.h"
#include "models/common/enummap.h"
#include "models/common/idstring.h"
#include "models/common/image.h"
#include "models/common/namedlist.h"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace UnTech {
class ErrorList;
}

namespace UnTech::MetaSprite::SpriteImporter {

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

    [[nodiscard]] inline urect cell(const upoint& gridLocation) const
    {
        return {
            gridLocation.x * (frameSize.width + padding.x) + offset.x,
            gridLocation.y * (frameSize.height + padding.y) + offset.y,
            frameSize.width,
            frameSize.height
        };
    }

    bool operator==(const FrameSetGrid&) const = default;
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

    [[nodiscard]] inline unsigned sizePx() const { return size == ObjectSize::SMALL ? 8 : 16; }

    [[nodiscard]] inline upoint bottomRight() const
    {
        return {
            location.x + sizePx(),
            location.y + sizePx()
        };
    }

    bool operator==(const FrameObject&) const = default;
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

    bool operator==(const ActionPoint&) const = default;
};

struct CollisionBox {
    urect aabb;
    bool exists;

    CollisionBox()
        : aabb(0, 0, MIN_FRAME_SIZE, MIN_FRAME_SIZE)
        , exists(false)
    {
    }

    bool operator==(const CollisionBox&) const = default;
};

struct Frame {
    idstring name;

    upoint gridLocation;
    std::optional<urect> locationOverride;
    std::optional<upoint> originOverride;

    std::vector<FrameObject> objects;
    std::vector<ActionPoint> actionPoints;
    SpriteOrderType spriteOrder = DEFAULT_SPRITE_ORDER;

    CollisionBox tileHitbox;
    CollisionBox shield;
    CollisionBox hitbox;
    CollisionBox hurtbox;

    [[nodiscard]] usize minimumViableSize(const FrameSetGrid& grid) const;

    [[nodiscard]] urect frameLocation(const FrameSetGrid& grid) const
    {
        if (!locationOverride) {
            return grid.cell(gridLocation);
        }
        else {
            return locationOverride.value();
        }
    }

    [[nodiscard]] upoint origin(const FrameSetGrid& grid) const
    {
        return originOverride.value_or(grid.origin);
    }

    bool operator==(const Frame&) const = default;
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

    [[nodiscard]] bool usesUserSuppliedPalette() const
    {
        return nPalettes > 0 && colorSize > 0;
    }

    [[nodiscard]] usize paletteSize() const
    {
        return { colorSize * PALETTE_COLORS, nPalettes * colorSize };
    }

    bool operator==(const UserSuppliedPalette&) const = default;
};

struct FrameSet {
    static const std::u8string FILE_EXTENSION;

    idstring name;
    TilesetType tilesetType = TilesetType::ONE_ROW;
    idstring exportOrder;
    NamedList<Frame> frames;
    NamedList<Animation::Animation> animations;

    std::filesystem::path imageFilename;
    UnTech::rgba transparentColor = rgba(0, 0, 0, 0);
    UserSuppliedPalette palette;
    FrameSetGrid grid;

    FrameSet() = default;

    bool operator==(const FrameSet&) const = default;
};

bool validate(const FrameSet& input, ErrorList& errorList);

std::unique_ptr<FrameSet> loadFrameSet(const std::filesystem::path& filename);
void saveFrameSet(const FrameSet& frameSet, const std::filesystem::path& filename);

}
