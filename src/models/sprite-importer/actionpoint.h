#ifndef _UNTECH_MODELS_SPRITEIMPORTER_ACTIONPOINT_H
#define _UNTECH_MODELS_SPRITEIMPORTER_ACTIONPOINT_H

#include "../common/aabb.h"
#include "../common/orderedlist.h"
#include <cstdint>
#include <memory>

namespace UnTech {
namespace SpriteImporter {

class Frame;

class ActionPoint {

public:
    typedef OrderedList<Frame, ActionPoint> list_t;

    typedef uint8_t parameter_t;

public:
    ActionPoint() = delete;
    ActionPoint(std::shared_ptr<Frame> frame);
    ActionPoint(const ActionPoint& point, std::shared_ptr<Frame> frame);

    std::shared_ptr<ActionPoint> clone(std::shared_ptr<Frame> frame);

    upoint location() const { return _location; }
    parameter_t parameter() const { return _parameter; }

    inline std::shared_ptr<Frame> frame() const { return _frame.lock(); }

    void setLocation(const upoint& location);
    void setParameter(parameter_t parameter);

private:
    std::weak_ptr<Frame> _frame;
    upoint _location;
    parameter_t _parameter;
};
}
}

#endif
