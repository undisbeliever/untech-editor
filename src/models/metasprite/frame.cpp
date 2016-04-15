#include "frame.h"
#include "frameobject.h"
#include "actionpoint.h"
#include "entityhitbox.h"
#include <algorithm>

using namespace UnTech;
using namespace UnTech::MetaSprite;

Frame::Frame(std::shared_ptr<FrameSet> frameSet)
    : _frameSet(frameSet)
    , _objects(*this)
    , _actionPoints(*this)
    , _entityHitboxes(*this)
    , _solid(true)
    , _tileHitbox(-8, -8, 16, 16)
{
}

Frame::Frame(const Frame& frame, std::shared_ptr<FrameSet> frameSet)
    : _frameSet(frameSet)
    , _objects(*this)
    , _actionPoints(*this)
    , _entityHitboxes(*this)
    , _solid(frame._solid)
    , _tileHitbox(frame._tileHitbox)
{
}

// Have to do this outside the constructor or else I get
// a bad_weak_ref exception
std::shared_ptr<Frame> Frame::clone(std::shared_ptr<FrameSet> frameSet)
{
    std::shared_ptr<Frame> frame(new Frame(*this, frameSet));

    for (const auto obj : this->_objects) {
        frame->_objects.clone(obj);
    }
    for (const auto ap : this->_actionPoints) {
        frame->_actionPoints.clone(ap);
    }
    for (const auto eh : this->_entityHitboxes) {
        frame->_entityHitboxes.clone(eh);
    }

    return frame;
}

Frame::Boundary Frame::calcBoundary() const
{
    // These numbers are selected so that origin (0, 0) is always visible.
    int left = -1;
    int right = 1;
    int top = -1;
    int bottom = 1;

    for (const auto obj : _objects) {
        const auto& loc = obj->location();
        const int size = obj->sizePx();

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

    for (const auto ap : _actionPoints) {
        const auto& loc = ap->location();

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
    for (const auto eh : _entityHitboxes) {
        const auto& aabb = eh->aabb();

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
