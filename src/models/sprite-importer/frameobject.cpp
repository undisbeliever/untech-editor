#include "frameobject.h"
#include "frame.h"

using namespace UnTech;
using namespace SpriteImporter;

FrameObject::FrameObject(std::shared_ptr<Frame> frame)
    : _frame(frame)
    , _location({ 0, 0 })
    , _size(ObjectSize::SMALL)
{
    const auto frameLocation = frame->location();

    _location.x = (frameLocation.width - sizePx()) / 2;
    _location.y = (frameLocation.height - sizePx()) / 2;
}

FrameObject::FrameObject(const FrameObject& object, std::shared_ptr<Frame> frame)
    : _frame(frame)
    , _location(object._location)
    , _size(object._size)
{
}

std::shared_ptr<FrameObject> FrameObject::clone(std::shared_ptr<Frame> frame)
{
    return std::make_shared<FrameObject>(*this, frame);
}

void FrameObject::setLocation(const upoint& location)
{
    auto frame = _frame.lock();

    if (frame) {
        upoint newLocation = frame->location().clipInside(location, sizePx());

        if (_location != newLocation) {
            _location = newLocation;
        }
    }
}

void FrameObject::setSize(ObjectSize size)
{
    auto frame = _frame.lock();
    if (frame) {
        if (_size != size) {
            _size = size;

            if (size == ObjectSize::LARGE) {
                // may now be outside frame.
                _location = frame->location().clipInside(_location, sizePx());
            }
        }
    }
}
