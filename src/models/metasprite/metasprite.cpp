/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metasprite.h"
#include "errorlisthelpers.h"
#include "models/common/errorlist.h"
#include "models/common/iterators.h"
#include "models/common/validateunique.h"
#include "models/metasprite/animation/animation.hpp"
#include <algorithm>

namespace UnTech::MetaSprite::MetaSprite {

/*
 * FRAME
 * =====
 */

static bool validate(const Frame& input, const unsigned frameIndex,
                     const FrameSet& fs, const ActionPointMapping& actionPointMapping,
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

    for (auto [i, obj] : const_enumerate(input.objects)) {
        size_t tsSize = obj.size == ObjectSize::SMALL ? fs.smallTileset.size()
                                                      : fs.largeTileset.size();
        if (obj.tileId > tsSize) {
            errorList.addError(frameObjectError(input, frameIndex, i, u8"Invalid tileId"));
            valid = false;
        }
    }

    for (auto [i, ap] : const_enumerate(input.actionPoints)) {
        if (actionPointMapping.find(ap.type) == actionPointMapping.end()) {
            errorList.addError(actionPointError(input, frameIndex, i, u8"Unknown action point type ", ap.type));
            valid = false;
        }
    }

    if (input.tileHitbox.exists) {
        if (input.tileHitbox.aabb.left() >= 0 || input.tileHitbox.aabb.right() <= 0
            || input.tileHitbox.aabb.top() >= 0 || input.tileHitbox.aabb.bottom() <= 0) {
            addError(u8"Frame origin must be inside the tile hitbox and not touching the hitbox edges");
        }
    }

    auto validateCollisionBox = [&](const CollisionBox& box, const std::u8string_view boxName, const MsErrorType type) {
        if (box.aabb.width == 0 || box.aabb.height == 0) {
            errorList.addError(collisionBoxError(input, frameIndex, type, boxName, u8" has no size"));
            valid = false;
        }
        else if (input.tileHitbox.aabb.left() < -127 || input.tileHitbox.aabb.right() > 127
                 || input.tileHitbox.aabb.top() < -127 || input.tileHitbox.aabb.height > 127) {

            errorList.addError(collisionBoxError(input, frameIndex, type, boxName, u8"is too large"));
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

Frame Frame::flip(bool hFlip, bool vFlip) const
{
    Frame ret(*this);

    for (auto& obj : ret.objects) {
        obj.location = obj.location.flip(hFlip, vFlip, obj.sizePx());
        obj.hFlip = obj.hFlip ^ hFlip;
        obj.vFlip = obj.vFlip ^ vFlip;
    }

    for (auto& ap : ret.actionPoints) {
        ap.location = ap.location.flip(hFlip, vFlip);
    }

    ret.tileHitbox.aabb = tileHitbox.aabb.flip(hFlip, vFlip);
    ret.shield.aabb = shield.aabb.flip(hFlip, vFlip);
    ret.hitbox.aabb = hitbox.aabb.flip(hFlip, vFlip);
    ret.hurtbox.aabb = hurtbox.aabb.flip(hFlip, vFlip);

    return ret;
}

/*
 * FRAME SET
 * =========
 */

bool validate(const FrameSet& input, const ActionPointMapping& actionPointMapping, ErrorList& errorList)
{
    bool valid = true;

    auto addError = [&](std::u8string&& msg) {
        errorList.addErrorString(std::move(msg));
        valid = false;
    };

    if (input.name.isValid() == false) {
        addError(u8"Missing name");
    }
    if (input.exportOrder.isValid() == false) {
        addError(u8"Missing exportOrder");
    }
    if (input.frames.size() == 0) {
        addError(u8"No Frames");
    }

    if (input.exportOrder.isValid() == false) {
        addError(u8"Missing exportOrder");
    }

    if (input.palettes.empty()) {
        addError(u8"Expected at least one palette");
    }
    if (input.palettes.size() > MAX_PALETTES) {
        addError(u8"Too many palettes");
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
        valid &= validate(frame, i, input, actionPointMapping, errorList);
    }

    for (auto [i, ani] : enumerate(input.animations)) {
        valid &= validate(ani, i, input, errorList);
    }

    return valid;
}

}
