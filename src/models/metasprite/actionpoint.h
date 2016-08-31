#pragma once

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
    static const char* TYPE_NAME;

    typedef uint8_t parameter_t;

public:
    ActionPoint() = delete;
    ActionPoint(const ActionPoint&) = delete;

    ActionPoint(Frame& frame)
        : _frame(frame)
        , _location(0, 0)
        , _parameter(1)
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
    inline void setParameter(parameter_t parameter)
    {
        if (parameter >= 1) {
            _parameter = parameter;
        }
    };

private:
    Frame& _frame;
    ms8point _location;
    parameter_t _parameter;
};
}
}
