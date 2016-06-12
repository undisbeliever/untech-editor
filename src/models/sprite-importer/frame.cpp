#include "frame.h"
#include "actionpoint.h"
#include "entityhitbox.h"
#include "frameobject.h"
#include <algorithm>

using namespace UnTech;
using namespace UnTech::SpriteImporter;

const char* Frame::TYPE_NAME = "FrameSet";

const usize Frame::MIN_SIZE = usize(Frame::MIN_WIDTH, Frame::MIN_HEIGHT);

Frame::Frame(FrameSet& frameSet)
    : _frameSet(frameSet)
    , _objects(*this)
    , _actionPoints(*this)
    , _entityHitboxes(*this)
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
    , _objects(*this)
    , _actionPoints(*this)
    , _entityHitboxes(*this)
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
    if (_useGridLocation != useGridLocation) {
        _useGridLocation = useGridLocation;

        if (useGridLocation == true) {
            recalculateLocation();
        }
    }
}

void Frame::setGridLocation(upoint gridLocation)
{
    if (_gridLocation != gridLocation) {
        _useGridLocation = true;
        _gridLocation = gridLocation;

        recalculateLocation();
    }
}

void Frame::setLocation(const urect& location)
{
    usize limit = minimumViableSize();

    if (_useGridLocation == false) {
        urect newLocation = location;

        newLocation.width = std::max(location.width, limit.width);
        newLocation.height = std::max(location.height, limit.height);

        if (_location != newLocation) {
            _location = newLocation;
        }
    }
}

void Frame::setUseGridOrigin(bool useGridOrigin)
{
    if (_useGridOrigin != useGridOrigin) {
        _useGridOrigin = useGridOrigin;

        if (useGridOrigin == true) {
            recalculateOrigin();
        }
    }
}

void Frame::setOrigin(upoint origin)
{
    if (_origin != origin) {
        if (_useGridOrigin == false) {
            _origin = _location.clipInside(origin);
        }
    }
}

void Frame::setSolid(bool solid)
{
    if (_solid != solid) {
        _solid = solid;
    }
}

void Frame::setTileHitbox(const urect& tileHitbox)
{
    urect newHitbox = _location.clipInside(tileHitbox, _tileHitbox);

    if (_tileHitbox != newHitbox) {
        _tileHitbox = newHitbox;
    }
}

void Frame::setSpriteOrder(unsigned spriteOrder)
{
    unsigned newOrder = spriteOrder & SPRITE_ORDER_MASK;

    if (_spriteOrder != newOrder) {
        _spriteOrder = newOrder;
    }
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
    const auto& grid = _frameSet.grid();

    if (_useGridLocation) {
        _location.x = _gridLocation.x * (grid.frameSize().width + grid.padding().width) + grid.offset().x;
        _location.y = _gridLocation.y * (grid.frameSize().height + grid.padding().height) + grid.offset().y;
        _location.width = grid.frameSize().width;
        _location.height = grid.frameSize().height;
    }
}

void Frame::recalculateOrigin()
{
    const auto& grid = _frameSet.grid();

    if (_useGridOrigin) {
        _origin = grid.origin();
    }
}
