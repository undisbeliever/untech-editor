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

public:
    ActionPoint() = delete;
    ActionPoint(const ActionPoint&) = delete;

    ActionPoint(std::shared_ptr<Frame> frame)
        : _frame(frame)
        , _location(0, 0)
        , _parameter(0)
    {
    }

    ActionPoint(const ActionPoint& point, std::shared_ptr<Frame> frame)
        : _frame(frame)
        , _location(point._location)
        , _parameter(point._parameter)
    {
    }

    std::shared_ptr<ActionPoint> clone(std::shared_ptr<Frame> frame)
    {
        return std::make_shared<ActionPoint>(*this, frame);
    }

    inline ms8point location() const { return _location; }
    inline parameter_t parameter() const { return _parameter; }

    inline std::shared_ptr<Frame> frame() const { return _frame.lock(); }
    inline MetaSpriteDocument& document() const { return frame()->frameSet()->document(); }

    inline void setLocation(const ms8point& location) { _location = location; };
    inline void setParameter(parameter_t parameter) { _parameter = parameter; };

private:
    std::weak_ptr<Frame> _frame;
    ms8point _location;
    parameter_t _parameter;
};
}
}

#endif
