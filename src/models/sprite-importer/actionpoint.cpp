#include "actionpoint.h"

using namespace UnTech;
using namespace SpriteImporter;

const char* ActionPoint::TYPE_NAME = "Action Point";

ActionPoint::ActionPoint(Frame& frame)
    : _frame(frame)
    , _location(0, 0)
    , _parameter(0)
{
    const auto frameLocation = frame.location();

    _location.x = frameLocation.width / 2;
    _location.y = frameLocation.height / 2;
}

ActionPoint::ActionPoint(const ActionPoint& point, Frame& frame)
    : _frame(frame)
    , _location(point._location)
    , _parameter(point._parameter)
{
}

void ActionPoint::setLocation(const upoint& location)
{
    upoint newLocation = _frame.location().clipInside(location);

    if (_location != newLocation) {
        _location = newLocation;
    }
}

void ActionPoint::setParameter(parameter_t parameter)
{
    if (_parameter != parameter) {
        _parameter = parameter;
    }
}
