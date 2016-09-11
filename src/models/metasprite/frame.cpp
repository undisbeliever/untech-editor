#include "frame.h"
#include "actionpoint.h"
#include "entityhitbox.h"
#include "frameobject.h"
#include "palette.h"
#include "models/metasprite-common/limits.h"
#include "models/snes/tileset.hpp"

using namespace UnTech;
using namespace UnTech::MetaSprite;
using namespace UnTech::MetaSpriteCommon;

Frame::Frame(FrameSet& frameSet)
    : _frameSet(frameSet)
    , _objects(*this, MAX_FRAME_OBJECTS)
    , _actionPoints(*this, MAX_ACTION_POINTS)
    , _entityHitboxes(*this, MAX_ENTITY_HITBOXES)
    , _solid(true)
    , _tileHitbox(-8, -8, 16, 16)
{
}

Frame::Frame(const Frame& frame, FrameSet& frameSet)
    : _frameSet(frameSet)
    , _objects(*this, MAX_FRAME_OBJECTS)
    , _actionPoints(*this, MAX_ACTION_POINTS)
    , _entityHitboxes(*this, MAX_ENTITY_HITBOXES)
    , _solid(frame._solid)
    , _tileHitbox(frame._tileHitbox)
{
    for (const auto& obj : frame._objects) {
        _objects.clone(obj);
    }
    for (const auto& ap : frame._actionPoints) {
        _actionPoints.clone(ap);
    }
    for (const auto& eh : frame._entityHitboxes) {
        _entityHitboxes.clone(eh);
    }
}

std::unique_ptr<Frame> Frame::flip(bool hFlip, bool vFlip) const
{
    std::unique_ptr<Frame> ret = std::make_unique<Frame>(*this, this->_frameSet);

    {
        ms8rect r = ret->_tileHitbox;

        if (hFlip) {
            r.x = -r.x - r.width;
        }
        if (vFlip) {
            r.y = -r.y - r.height;
        }

        ret->_tileHitbox = r;
    }

    for (auto& obj : ret->_objects) {
        ms8point p = obj.location();

        if (hFlip) {
            p.x = -p.x - obj.sizePx();
            obj.setHFlip(!obj.hFlip());
        }
        if (vFlip) {
            p.y = -p.y - obj.sizePx();
            obj.setVFlip(!obj.vFlip());
        }

        obj.setLocation(p);
    }

    for (auto& ap : ret->_actionPoints) {
        ms8point p = ap.location();

        if (hFlip) {
            p.x = -p.x;
        }
        if (vFlip) {
            p.y = -p.y;
        }

        ap.setLocation(p);
    }

    for (auto& eh : ret->_entityHitboxes) {
        ms8rect r = eh.aabb();

        if (hFlip) {
            r.x = -r.x - r.width;
        }
        if (vFlip) {
            r.y = -r.y - r.height;
        }

        eh.setAabb(r);
    }

    return ret;
}

Frame::Boundary Frame::calcBoundary() const
{
    // These numbers are selected so that origin (0, 0) is always visible.
    int left = -1;
    int right = 1;
    int top = -1;
    int bottom = 1;

    for (const FrameObject& obj : _objects) {
        const auto& loc = obj.location();
        const int size = obj.sizePx();

        if (loc.x < left) {
            left = loc.x;
        }
        if (loc.x + size > right) {
            right = loc.x + size;
        }
        if (loc.y < top) {
            top = loc.y;
        }
        if (loc.y + size > bottom) {
            bottom = loc.y + size;
        }
    }

    for (const ActionPoint& ap : _actionPoints) {
        const auto& loc = ap.location();

        if (loc.x < left) {
            left = loc.x;
        }
        if (loc.x > right) {
            right = loc.x;
        }
        if (loc.y < top) {
            top = loc.y;
        }
        if (loc.y > bottom) {
            bottom = loc.y;
        }
    }
    for (const EntityHitbox& eh : _entityHitboxes) {
        const auto& aabb = eh.aabb();

        if (aabb.x < left) {
            left = aabb.x;
        }
        if (aabb.right() > right) {
            right = aabb.right();
        }
        if (aabb.y < top) {
            top = aabb.y;
        }
        if (aabb.bottom() > bottom) {
            bottom = aabb.bottom();
        }
    }

    return { left, top,
             (unsigned)right - left,
             (unsigned)top - bottom };
}

void Frame::draw(Image& image, const Palette& palette, unsigned xOffset, unsigned yOffset) const
{
    // Ignore sprite order.
    //
    // The SNES will only check the priority of the topmost OAM object
    // (for the current pixel) and ignore the priority of all of the
    // other objects.

    for (auto it = _objects.rbegin(); it != _objects.rend(); ++it) {
        const FrameObject& obj = *it;

        if (obj.size() == FrameObject::ObjectSize::SMALL) {
            _frameSet.smallTileset().drawTile(image, palette,
                                              xOffset + obj.location().x, yOffset + obj.location().y,
                                              obj.tileId(), obj.hFlip(), obj.vFlip());
        }
        else {
            _frameSet.largeTileset().drawTile(image, palette,
                                              xOffset + obj.location().x, yOffset + obj.location().y,
                                              obj.tileId(), obj.hFlip(), obj.vFlip());
        }
    }
}
