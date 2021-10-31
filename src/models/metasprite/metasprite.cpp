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
        addError("Missing name");
    }
    if (input.objects.size() > MAX_FRAME_OBJECTS) {
        addError("Too many frame objects");
    }
    if (input.actionPoints.size() > MAX_ACTION_POINTS) {
        addError("Too many action points");
    }

    for (auto [i, obj] : const_enumerate(input.objects)) {
        size_t tsSize = obj.size == ObjectSize::SMALL ? fs.smallTileset.size()
                                                      : fs.largeTileset.size();
        if (obj.tileId > tsSize) {
            errorList.addError(frameObjectError(input, frameIndex, i, "Invalid tileId"));
            valid = false;
        }
    }

    for (auto [i, ap] : const_enumerate(input.actionPoints)) {
        if (actionPointMapping.find(ap.type) == actionPointMapping.end()) {
            errorList.addError(actionPointError(input, frameIndex, i, "Unknown action point type ", ap.type));
            valid = false;
        }
    }

    if (input.tileHitbox.exists) {
        if (input.tileHitbox.aabb.left() >= 0 || input.tileHitbox.aabb.right() <= 0
            || input.tileHitbox.aabb.top() >= 0 || input.tileHitbox.aabb.bottom() <= 0) {
            addError("Frame origin must be inside the tile hitbox and not touching the hitbox edges");
        }
    }

    auto validateCollisionBox = [&](const CollisionBox& box, const std::string_view boxName, const MsErrorType type) {
        if (box.aabb.width == 0 || box.aabb.height == 0) {
            errorList.addError(collisionBoxError(input, frameIndex, type, boxName, " has no size"));
            valid = false;
        }
        else if (input.tileHitbox.aabb.left() < -127 || input.tileHitbox.aabb.right() > 127
                 || input.tileHitbox.aabb.top() < -127 || input.tileHitbox.aabb.height > 127) {

            errorList.addError(collisionBoxError(input, frameIndex, type, boxName, "is too large"));
            valid = false;
        }
        else if (box.aabb.width >= MAX_COLLISION_BOX_SIZE || box.aabb.height >= MAX_COLLISION_BOX_SIZE) {
            errorList.addError(collisionBoxError(input, frameIndex, type, boxName, " is too large"));
            valid = false;
        }
    };
    validateCollisionBox(input.tileHitbox, "Tile Hitbox", MsErrorType::TILE_HITBOX);
    validateCollisionBox(input.shield, "Shield", MsErrorType::SHIELD);
    validateCollisionBox(input.hitbox, "Hitbox", MsErrorType::HIT_BOX);
    validateCollisionBox(input.hurtbox, "Hurtbox", MsErrorType::HURT_BOX);

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

    auto addError = [&](std::string&& msg) {
        errorList.addErrorString(std::move(msg));
        valid = false;
    };

    if (input.name.isValid() == false) {
        addError("Missing name");
    }
    if (input.exportOrder.isValid() == false) {
        addError("Missing exportOrder");
    }
    if (input.frames.size() == 0) {
        addError("No Frames");
    }

    if (input.exportOrder.isValid() == false) {
        addError("Missing exportOrder");
    }

    if (input.palettes.empty()) {
        addError("Expected at least one palette");
    }
    if (input.palettes.size() > MAX_PALETTES) {
        addError("Too many palettes");
    }
    if (input.animations.size() > MAX_ANIMATION_FRAMES) {
        addError("Too many animations in frameSet");
    }

    valid &= validateNamesUnique(input.frames, "frame", [&](unsigned i, auto... msg) {
        errorList.addError(std::make_unique<MetaSpriteError>(MsErrorType::FRAME, i, stringBuilder(msg...)));
    });
    valid &= validateNamesUnique(input.animations, "animation", [&](unsigned i, auto... msg) {
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
