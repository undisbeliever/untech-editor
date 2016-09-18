#include "metasprite.h"
#include "models/snes/tile.hpp"
#include "models/snes/tileset.hpp"
#include <algorithm>

using namespace UnTech;
using namespace UnTech::MetaSprite;
using namespace UnTech::MetaSprite::MetaSprite;

template <>
const std::string FrameObject::list_t::HUMAN_TYPE_NAME = "Frame Object";
template <>
const std::string ActionPoint::list_t::HUMAN_TYPE_NAME = "Action Point";
template <>
const std::string EntityHitbox::list_t::HUMAN_TYPE_NAME = "Entity Hitbox";

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

    {
        if (hFlip) {
            ret.tileHitbox.x = -ret.tileHitbox.x - ret.tileHitbox.width;
        }
        if (vFlip) {
            ret.tileHitbox.y = -ret.tileHitbox.y - ret.tileHitbox.height;
        }
    }

    for (auto& obj : ret.objects) {
        if (hFlip) {
            obj.location.x = -obj.location.x - obj.sizePx();
            obj.hFlip = !obj.hFlip;
        }
        if (vFlip) {
            obj.location.y = -obj.location.y - obj.sizePx();
            obj.vFlip = !obj.vFlip;
        }
    }

    for (auto& ap : ret.actionPoints) {
        if (hFlip) {
            ap.location.x = -ap.location.x;
        }
        if (vFlip) {
            ap.location.y = -ap.location.y;
        }
    }

    for (auto& eh : ret.entityHitboxes) {
        if (hFlip) {
            eh.aabb.x = -eh.aabb.x - eh.aabb.width;
        }
        if (vFlip) {
            eh.aabb.y = -eh.aabb.y - eh.aabb.height;
        }
    }

    return ret;
}

void Frame::draw(Image& image, const FrameSet& frameSet, size_t paletteId,
                 unsigned xOffset, unsigned yOffset) const
{
    // Ignore sprite order.
    //
    // The SNES will only check the priority of the topmost OAM object
    // (for the current pixel) and ignore the priority of all of the
    // other objects.

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
