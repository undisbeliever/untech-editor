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
