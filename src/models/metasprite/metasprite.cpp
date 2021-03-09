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
#include <algorithm>

using namespace UnTech;
using namespace UnTech::MetaSprite;
using namespace UnTech::MetaSprite::MetaSprite;

/*
 * FRAME
 * =====
 */

inline bool Frame::validate(const ActionPointMapping& actionPointMapping, ErrorList& errorList, const FrameSet& fs) const
{
    bool valid = true;
    auto addError = [&](const auto... msg) {
        errorList.addError(frameError(*this, msg...));
        valid = false;
    };

    if (!name.isValid()) {
        addError("Missing name");
    }
    if (objects.size() > MAX_FRAME_OBJECTS) {
        addError("Too many frame objects");
    }
    if (actionPoints.size() > MAX_ACTION_POINTS) {
        addError("Too many action points");
    }
    if (entityHitboxes.size() > MAX_ENTITY_HITBOXES) {
        addError("Too many entity hitboxes");
    }

    for (auto [i, obj] : const_enumerate(objects)) {
        size_t tsSize = obj.size == ObjectSize::SMALL ? fs.smallTileset.size()
                                                      : fs.largeTileset.size();
        if (obj.tileId > tsSize) {
            errorList.addError(frameObjectError(*this, i, "Invalid tileId"));
            valid = false;
        }
    }

    for (auto [i, ap] : const_enumerate(actionPoints)) {
        if (actionPointMapping.find(ap.type) == actionPointMapping.end()) {
            errorList.addError(actionPointError(*this, i, "Unknown action point type ", ap.type));
            valid = false;
        }
    }

    for (auto [i, eh] : const_enumerate(entityHitboxes)) {
        if (eh.aabb.width == 0 || eh.aabb.height == 0) {
            errorList.addError(entityHitboxError(*this, i, "aabb has no size"));
            valid = false;
        }
    }

    if (solid) {
        if (tileHitbox.width == 0 || tileHitbox.height == 0) {
            addError("Tile Hitbox has no size");
        }
        else if (tileHitbox.left() >= 0 || tileHitbox.right() <= 0
                 || tileHitbox.top() >= 0 || tileHitbox.bottom() <= 0) {
            addError("Frame origin must be inside the tile hitbox and not touching the hitbox edges");
        }

        if (tileHitbox.left() < -127 || tileHitbox.right() > 127
            || tileHitbox.top() < -127 || tileHitbox.height > 127) {
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

void Frame::draw(Image& image, const FrameSet& frameSet, size_t paletteId,
                 unsigned xOffset, unsigned yOffset) const
{
    const Snes::Palette4bpp& palette = frameSet.palettes[paletteId];

    for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
        const FrameObject& obj = *it;

        if (obj.size == ObjectSize::SMALL) {
            frameSet.smallTileset.drawTile(image, palette,
                                           xOffset + obj.location.x, yOffset + obj.location.y,
                                           obj.tileId, obj.hFlip, obj.vFlip);
        }
        else {
            frameSet.largeTileset.drawTile(image, palette,
                                           xOffset + obj.location.x, yOffset + obj.location.y,
                                           obj.tileId, obj.hFlip, obj.vFlip);
        }
    }
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

bool FrameSet::validate(const ActionPointMapping& actionPointMapping, ErrorList& errorList) const
{
    bool valid = true;

    auto addError = [&](std::string&& msg) {
        errorList.addErrorString(msg);
        valid = false;
    };

    if (name.isValid() == false) {
        addError("Missing name");
    }
    if (exportOrder.isValid() == false) {
        addError("Missing exportOrder");
    }
    if (frames.size() == 0) {
        addError("No Frames");
    }

    if (exportOrder.isValid() == false) {
        addError("Missing exportOrder");
    }

    if (palettes.empty()) {
        addError("Expected at least one palette");
    }
    if (palettes.size() > MAX_PALETTES) {
        addError("Too many palettes");
    }
    if (animations.size() > MAX_ANIMATION_FRAMES) {
        addError("Too many animations in frameSet");
    }

    valid &= validateNamesUnique(frames, "frame", errorList);
    valid &= validateNamesUnique(animations, "animation", errorList);

    for (auto& frame : frames) {
        valid &= frame.validate(actionPointMapping, errorList, *this);
    }

    for (auto& ani : animations) {
        valid &= ani.validate(*this, errorList);
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
