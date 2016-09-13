#include "frame.h"
#include "actionpoint.h"
#include "entityhitbox.h"
#include "frameobject.h"
#include "models/metasprite-common/limits.h"
#include <algorithm>

using namespace UnTech;
using namespace UnTech::SpriteImporter;
using namespace UnTech::MetaSpriteCommon;

const char* Frame::TYPE_NAME = "Frame";

const usize Frame::MIN_SIZE = usize(Frame::MIN_WIDTH, Frame::MIN_HEIGHT);

Frame::Frame(FrameSet& frameSet)
    : _frameSet(frameSet)
    , _objects(*this, MAX_FRAME_OBJECTS)
    , _actionPoints(*this, MAX_ACTION_POINTS)
    , _entityHitboxes(*this, MAX_ENTITY_HITBOXES)
    , _useGridLocation(true)
    , _gridLocation(0, 0)
    , _useGridOrigin(true)
    , _solid(true)
    , _tileHitbox(0, 0, MIN_WIDTH, MIN_HEIGHT)
    , _spriteOrder(DEFAULT_SPRITE_ORDER)
{
    recalculateLocation();
    recalculateOrigin();

    _tileHitbox.x = (_location.width - _tileHitbox.width) / 2;
    _tileHitbox.y = (_location.height - _tileHitbox.height) / 2;
}

Frame::Frame(const Frame& frame, FrameSet& frameSet)
    : _frameSet(frameSet)
    , _objects(*this, MAX_FRAME_OBJECTS)
    , _actionPoints(*this, MAX_ACTION_POINTS)
    , _entityHitboxes(*this, MAX_ENTITY_HITBOXES)
    , _useGridLocation(frame._useGridLocation)
    , _gridLocation(frame._gridLocation)
    , _location(frame._location)
    , _useGridOrigin(frame._useGridOrigin)
    , _origin(frame._origin)
    , _solid(frame._solid)
    , _tileHitbox(frame._tileHitbox)
    , _spriteOrder(frame._spriteOrder)
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

void Frame::setUseGridLocation(bool useGridLocation)
{
    _useGridLocation = useGridLocation;
    recalculateLocation();
}

void Frame::setGridLocation(upoint gridLocation)
{
    _useGridLocation = true;
    _gridLocation = gridLocation;
    recalculateLocation();
}

void Frame::setLocation(const urect& location)
{
    _useGridLocation = true;

    urect newLocation = location;
    usize limit = minimumViableSize();
    newLocation.width = std::max(location.width, limit.width);
    newLocation.height = std::max(location.height, limit.height);

    _location = newLocation;
}

void Frame::setUseGridOrigin(bool useGridOrigin)
{
    _useGridOrigin = useGridOrigin;
    recalculateOrigin();
}

void Frame::setOrigin(upoint origin)
{
    _useGridOrigin = false;
    _origin = _location.clipInside(origin);
}

void Frame::setSolid(bool solid)
{
    _solid = solid;
}

void Frame::setTileHitbox(const urect& tileHitbox)
{
    _tileHitbox = _location.clipInside(tileHitbox, _tileHitbox);
}

void Frame::setSpriteOrder(unsigned spriteOrder)
{
    _spriteOrder = spriteOrder & SPRITE_ORDER_MASK;
}

usize Frame::minimumViableSize() const
{
    usize limit = usize(MIN_WIDTH, MIN_HEIGHT);

    limit = limit.expand(_origin);

    for (const auto& obj : _objects) {
        limit = limit.expand(obj.bottomLeft());
    }

    for (const auto& ap : _actionPoints) {
        limit = limit.expand(ap.location());
    }

    for (const auto& eh : _entityHitboxes) {
        limit = limit.expand(eh.aabb());
    }

    return limit;
}

void Frame::recalculateLocation()
{
    if (_useGridLocation) {
        const auto& grid = _frameSet.grid();

        _location.x = _gridLocation.x * (grid.frameSize().width + grid.padding().width) + grid.offset().x;
        _location.y = _gridLocation.y * (grid.frameSize().height + grid.padding().height) + grid.offset().y;
        _location.width = grid.frameSize().width;
        _location.height = grid.frameSize().height;
    }
}

void Frame::recalculateOrigin()
{
    if (_useGridOrigin) {
        _origin = _frameSet.grid().origin();
    }
}
