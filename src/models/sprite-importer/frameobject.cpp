#include "frameobject.h"

using namespace UnTech;
using namespace SpriteImporter;

FrameObject::FrameObject(Frame& frame)
    : _frame(frame)
    , _location(0, 0)
    , _size(ObjectSize::SMALL)
{
    const auto frameLocation = frame.location();

    _location.x = (frameLocation.width - sizePx()) / 2;
    _location.y = (frameLocation.height - sizePx()) / 2;
}

FrameObject::FrameObject(const FrameObject& object, Frame& frame)
    : _frame(frame)
    , _location(object._location)
    , _size(object._size)
{
}

void FrameObject::setLocation(const upoint& location)
{
    upoint newLocation = _frame.location().clipInside(location, sizePx());

    if (_location != newLocation) {
        _location = newLocation;
    }
}

void FrameObject::setSize(ObjectSize size)
{
    if (_size != size) {
        _size = size;

        if (size == ObjectSize::LARGE) {
            // may now be outside frame.
            _location = _frame.location().clipInside(_location, sizePx());
        }
    }
}
