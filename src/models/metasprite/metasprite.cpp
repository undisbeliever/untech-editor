/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "metasprite.h"
#include "models/common/humantypename.h"
#include <algorithm>

using namespace UnTech;
using namespace UnTech::MetaSprite;
using namespace UnTech::MetaSprite::MetaSprite;

template <>
const std::string HumanTypeName<FrameSet>::value = "Frame Set";
template <>
const std::string HumanTypeName<Frame>::value = "Frame";
template <>
const std::string HumanTypeName<FrameObject>::value = "Frame Object";
template <>
const std::string HumanTypeName<ActionPoint>::value = "Action Point";
template <>
const std::string HumanTypeName<EntityHitbox>::value = "Entity Hitbox";

/*
 * FRAME OBJECT
 * ============
 */

bool FrameObject::isValid(const FrameSet& frameSet)
{
    if (size == ObjectSize::SMALL) {
        return tileId < frameSet.smallTileset.size();
    }
    else {
        return tileId < frameSet.largeTileset.size();
    }
}

/*
 * FRAME
 * =====
 */

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
