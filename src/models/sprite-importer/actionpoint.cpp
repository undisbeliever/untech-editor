#include "actionpoint.h"

using namespace UnTech;
using namespace SpriteImporter;

const char* ActionPoint::TYPE_NAME = "Action Point";

ActionPoint::ActionPoint(Frame& frame)
    : _frame(frame)
    , _location(0, 0)
    , _parameter(1)
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
    _location = _frame.location().clipInside(location);
}

void ActionPoint::setParameter(parameter_t parameter)
{
    // parameter cannot be 0
    if (parameter >= 1) {
        _parameter = parameter;
    }
}
