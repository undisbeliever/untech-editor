#include "actionpoint.h"

using namespace UnTech;
using namespace SpriteImporter;

ActionPoint::ActionPoint(std::shared_ptr<Frame> frame)
    : _frame(frame)
    , _location(0, 0)
    , _parameter(0)
{
    const auto frameLocation = frame->location();

    _location.x = frameLocation.width / 2;
    _location.y = frameLocation.height / 2;
}

ActionPoint::ActionPoint(const ActionPoint& point, std::shared_ptr<Frame> frame)
    : _frame(frame)
    , _location(point._location)
    , _parameter(point._parameter)
{
}

std::shared_ptr<ActionPoint> ActionPoint::clone(std::shared_ptr<Frame> frame)
{
    return std::make_shared<ActionPoint>(*this, frame);
}

void ActionPoint::setLocation(const upoint& location)
{
    auto frame = _frame.lock();

    if (frame) {
        upoint newLocation = frame->location().clipInside(location);

        if (_location != newLocation) {
            _location = newLocation;
        }
    }
}

void ActionPoint::setParameter(parameter_t parameter)
{
    auto frame = _frame.lock();
    if (frame) {
        if (_parameter != parameter) {
            _parameter = parameter;
        }
    }
}
