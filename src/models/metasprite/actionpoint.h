#ifndef _UNTECH_MODELS_METASPRITE_ACTIONPOINT_H
#define _UNTECH_MODELS_METASPRITE_ACTIONPOINT_H

#include "frame.h"
#include "../common/ms8aabb.h"
#include "../common/orderedlist.h"
#include <cstdint>
#include <memory>

namespace UnTech {
namespace MetaSprite {

class ActionPoint {
public:
    typedef OrderedList<Frame, ActionPoint> list_t;

    typedef uint8_t parameter_t;

    const static unsigned MAX_ACTION_POINTS = 12;

public:
    ActionPoint() = delete;
    ActionPoint(const ActionPoint&) = delete;

    ActionPoint(Frame& frame)
        : _frame(frame)
        , _location(0, 0)
        , _parameter(0)
    {
    }

    ActionPoint(const ActionPoint& point, Frame& frame)
        : _frame(frame)
        , _location(point._location)
        , _parameter(point._parameter)
    {
    }

    inline Frame& frame() const { return _frame; }
    inline MetaSpriteDocument& document() const { return _frame.frameSet().document(); }

    inline ms8point location() const { return _location; }
    inline parameter_t parameter() const { return _parameter; }

    inline void setLocation(const ms8point& location) { _location = location; };
    inline void setParameter(parameter_t parameter) { _parameter = parameter; };

private:
    Frame& _frame;
    ms8point _location;
    parameter_t _parameter;
};
}
}

#endif
