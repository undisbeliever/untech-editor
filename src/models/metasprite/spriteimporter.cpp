/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "spriteimporter.h"
#include "errorlisthelpers.h"
#include "models/common/errorlist.h"
#include "models/common/file.h"
#include "models/common/imagecache.h"
#include "models/common/iterators.h"
#include "models/common/validateunique.h"
#include "models/metasprite/animation/animation.hpp"
#include <algorithm>

namespace UnTech::MetaSprite::SpriteImporter {

const EnumMap<UserSuppliedPalette::Position> UserSuppliedPalette::positionEnumMap{ {
    { u8"TOP_LEFT", UserSuppliedPalette::Position::TOP_LEFT },
    { u8"TOP_RIGHT", UserSuppliedPalette::Position::TOP_RIGHT },
    { u8"BOTTOM_LEFT", UserSuppliedPalette::Position::BOTTOM_LEFT },
    { u8"BOTTOM_RIGHT", UserSuppliedPalette::Position::BOTTOM_RIGHT },
} };

/*
 * FRAME SET GRID
 * ==============
 */

static bool validate(const FrameSetGrid& input, ErrorList& errorList)
{
    bool valid = true;
    auto addError = [&](const auto&... msg) {
        errorList.addErrorString(msg...);
        valid = false;
    };

    if (input.frameSize.width == 0 || input.frameSize.height == 0) {
        addError(u8"grid.frameSize has no size");
    }
    if (input.frameSize.width > MAX_FRAME_SIZE || input.frameSize.height > MAX_FRAME_SIZE) {
        addError(u8"grid.frameSize is too large (max: ", MAX_FRAME_SIZE, u8" x ", MAX_FRAME_SIZE, u8")");
    }
    if (input.origin.x > MAX_ORIGIN || input.origin.y > MAX_ORIGIN) {
        addError(u8"grid.origin is too large (max: ", MAX_ORIGIN, u8", u8", MAX_ORIGIN, u8")");
    }
    if (input.frameSize.contains(input.origin) == false) {
        addError(u8"grid.origin is not inside grid.frameSize");
    }

    return valid;
}

/*
 * FRAME
 * =====
 */

/*
 * NOTE: ActionPoint::type is only tested if actionPointMapping is not empty,
 *       which allows utsi2utms to work without specifing a project file.
 */
static bool validate(const Frame& input, const unsigned frameIndex, const FrameSetGrid& grid, const Image& image, const ActionPointMapping& actionPointMapping,
                     ErrorList& errorList)
{
    bool valid = true;

    auto addError = [&](const auto... msg) {
        errorList.addError(frameError(input, frameIndex, msg...));
        valid = false;
    };

    if (!input.name.isValid()) {
        addError(u8"Missing name");
    }
    if (input.objects.size() > MAX_FRAME_OBJECTS) {
        addError(u8"Too many frame objects");
    }
    if (input.actionPoints.size() > MAX_ACTION_POINTS) {
        addError(u8"Too many action points");
    }

    const auto aabb = input.frameLocation(grid);
    const auto origin = input.origin(grid);

    if (input.locationOverride) {
        if (aabb.width == 0 || aabb.height == 0) {
            addError(u8"frame location has no size");
        }
        if (aabb.width > MAX_FRAME_SIZE || aabb.height > MAX_FRAME_SIZE) {
            addError(u8"frame location is too large (", MAX_FRAME_SIZE, u8" x ", MAX_FRAME_SIZE, u8")");
        }
    }

    if (input.originOverride) {
        if (origin.x > MAX_ORIGIN || origin.y > MAX_ORIGIN) {
            addError(u8"origin is too large (max: ", MAX_ORIGIN, u8", u8", MAX_ORIGIN, u8")");
        }
        if (aabb.size().contains(origin) == false) {
            addError(u8"origin is not inside frame");
        }
    }

    if (image.size().contains(aabb) == false) {
        addError(u8"Frame not inside image");
    }

    if (valid == false) {
        return false;
    }

    const usize frameSize = aabb.size();

    for (auto [i, obj] : const_enumerate(input.objects)) {
        if (frameSize.contains(obj.location, obj.sizePx()) == false) {
            errorList.addError(frameObjectError(input, frameIndex, i, u8"Frame Object not inside frame"));
            valid = false;
        }
    }

    for (auto [i, ap] : const_enumerate(input.actionPoints)) {
        if (frameSize.contains(ap.location) == false) {
            errorList.addError(actionPointError(input, frameIndex, i, u8"location not inside frame"));
            valid = false;
        }

        if (not actionPointMapping.empty()) {
            if (actionPointMapping.find(ap.type) == actionPointMapping.end()) {
                errorList.addError(actionPointError(input, frameIndex, i, u8"Unknown action point type ", ap.type));
                valid = false;
            }
        }
    }

    if (input.tileHitbox.exists) {
        if (!input.tileHitbox.aabb.contains(origin)) {
            addError(u8"Frame origin must be inside the tile hitbox");
        }
    }

    auto validateCollisionBox = [&](const CollisionBox& box, const std::u8string_view boxName, const MsErrorType type) {
        if (frameSize.contains(box.aabb) == false) {
            errorList.addError(collisionBoxError(input, frameIndex, type, boxName, u8" not inside frame"));
            valid = false;
        }

        if (box.aabb.width == 0 || box.aabb.height == 0) {
            errorList.addError(collisionBoxError(input, frameIndex, type, boxName, u8" has no size"));
            valid = false;
        }
        else if (box.aabb.width >= MAX_COLLISION_BOX_SIZE || box.aabb.height >= MAX_COLLISION_BOX_SIZE) {
            errorList.addError(collisionBoxError(input, frameIndex, type, boxName, u8" is too large"));
            valid = false;
        }
    };
    validateCollisionBox(input.tileHitbox, u8"Tile Hitbox", MsErrorType::TILE_HITBOX);
    validateCollisionBox(input.shield, u8"Shield", MsErrorType::SHIELD);
    validateCollisionBox(input.hitbox, u8"Hitbox", MsErrorType::HIT_BOX);
    validateCollisionBox(input.hurtbox, u8"Hurtbox", MsErrorType::HURT_BOX);

    return valid;
}

usize Frame::minimumViableSize(const FrameSetGrid& grid) const
{
    usize limit = usize(MIN_FRAME_SIZE, MIN_FRAME_SIZE);

    limit = limit.expand(origin(grid));

    auto expandCollisionBox = [&](const CollisionBox& box) {
        if (box.exists) {
            limit = limit.expand(box.aabb);
        }
    };
    expandCollisionBox(tileHitbox);
    expandCollisionBox(shield);
    expandCollisionBox(hitbox);
    expandCollisionBox(hurtbox);

    for (const auto& obj : objects) {
        limit = limit.expand(obj.bottomRight());
    }

    for (const auto& ap : actionPoints) {
        limit = limit.expand(ap.location);
    }

    return limit;
}

/*
 * FRAME SET
 * =========
 */

static bool isTransparentColorValid(const FrameSet& input, const Image& image)
{
    if (input.transparentColor.alpha == 0xff) {
        return true;
    }
    if (input.transparentColor.alpha == 0) {
        const auto imgBits = image.data();

        bool hasTransparent = std::any_of(imgBits.begin(), imgBits.end(),
                                          [&](const rgba& c) { return c.alpha == 0; });

        return hasTransparent && input.transparentColor == rgba(0, 0, 0, 0);
    }
    else {
        return false;
    }
}

static bool validate(const FrameSet& input, const ActionPointMapping& actionPointMapping, ErrorList& errorList)
{
    bool valid = true;
    auto addError = [&](std::u8string&& msg) {
        errorList.addErrorString(std::move(msg));
        valid = false;
    };

    // Validate FrameSet

    if (input.name.isValid() == false) {
        addError(u8"Missing name");
    }
    if (input.exportOrder.isValid() == false) {
        addError(u8"Missing exportOrder");
    }
    if (input.frames.size() == 0) {
        addError(u8"No Frames");
    }
    if (input.imageFilename.empty()) {
        addError(u8"No Image");
    }
    valid &= validate(input.grid, errorList);

    if (valid == false) {
        return false;
    }

    const auto image = ImageCache::loadPngImage(input.imageFilename);
    assert(image);
    if (image->empty()) {
        addError(image->errorString());
        return false;
    }

    if (isTransparentColorValid(input, *image) == false) {
        addError(u8"Transparent color is invalid");
    }

    if (input.palette.usesUserSuppliedPalette()) {
        auto imgSize = image->size();
        auto palSize = input.palette.paletteSize();

        if (input.palette.nPalettes > MAX_PALETTES) {
            addError(u8"Too many palettes");
        }

        if (palSize.width > imgSize.width
            || palSize.height > imgSize.height) {

            addError(u8"Palette outside image");
        }
    }

    if (input.animations.size() > MAX_ANIMATION_FRAMES) {
        addError(u8"Too many animations in frameSet");
    }

    valid &= validateNamesUnique(input.frames, u8"frame", [&](unsigned i, auto... msg) {
        errorList.addError(std::make_unique<MetaSpriteError>(MsErrorType::FRAME, i, stringBuilder(msg...)));
    });
    valid &= validateNamesUnique(input.animations, u8"animation", [&](unsigned i, auto... msg) {
        errorList.addError(std::make_unique<MetaSpriteError>(MsErrorType::ANIMATION, i, stringBuilder(msg...)));
    });

    for (auto [i, frame] : enumerate(input.frames)) {
        valid &= validate(frame, i, input.grid, *image, actionPointMapping, errorList);
    }

    for (auto [i, ani] : enumerate(input.animations)) {
        valid &= validate(ani, i, input, errorList);
    }

    return valid;
}

// This function exists to allows utsi2utms to work without specifing a project file
bool validate(const FrameSet& input, ErrorList& errorList)
{
    return validate(input, ActionPointMapping{}, errorList);
}

}
