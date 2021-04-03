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
    if (input.entityHitboxes.size() > MAX_ENTITY_HITBOXES) {
        addError("Too many entity hitboxes");
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

    for (auto [i, eh] : const_enumerate(input.entityHitboxes)) {
        if (eh.aabb.width == 0 || eh.aabb.height == 0) {
            errorList.addError(entityHitboxError(input, frameIndex, i, "aabb has no size"));
            valid = false;
        }
    }

    if (input.solid) {
        if (input.tileHitbox.width == 0 || input.tileHitbox.height == 0) {
            addError("Tile Hitbox has no size");
        }
        else if (input.tileHitbox.left() >= 0 || input.tileHitbox.right() <= 0
                 || input.tileHitbox.top() >= 0 || input.tileHitbox.bottom() <= 0) {
            addError("Frame origin must be inside the tile hitbox and not touching the hitbox edges");
        }

        if (input.tileHitbox.left() < -127 || input.tileHitbox.right() > 127
            || input.tileHitbox.top() < -127 || input.tileHitbox.height > 127) {
            addError("Tile Hitbox is too large");
        }
    }

    return valid;
}

Frame Frame::flip(bool hFlip, bool vFlip) const
{
    Frame ret(*this);

    ret.tileHitbox = tileHitbox.flip(hFlip, vFlip);

    for (auto& obj : ret.objects) {
        obj.location = obj.location.flip(hFlip, vFlip, obj.sizePx());
        obj.hFlip = obj.hFlip ^ hFlip;
        obj.vFlip = obj.vFlip ^ vFlip;
    }

    for (auto& ap : ret.actionPoints) {
        ap.location = ap.location.flip(hFlip, vFlip);
    }

    for (auto& eh : ret.entityHitboxes) {
        eh.aabb = eh.aabb.flip(hFlip, vFlip);
    }

    return ret;
}

bool Frame::operator==(const Frame& o) const
{
    return objects == o.objects
           && actionPoints == o.actionPoints
           && entityHitboxes == o.entityHitboxes
           && spriteOrder == o.spriteOrder
           && tileHitbox == o.tileHitbox
           && solid == o.solid;
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

bool FrameSet::operator==(const FrameSet& o) const
{
    return name == o.name
           && tilesetType == o.tilesetType
           && exportOrder == o.exportOrder
           && smallTileset == o.smallTileset
           && largeTileset == o.largeTileset
           && palettes == o.palettes
           && frames == o.frames
           && animations == o.animations;
}

}
