#ifndef _UNTECH_MODELS_SPRITEIMPORTER_ACTIONPOINT_H
#define _UNTECH_MODELS_SPRITEIMPORTER_ACTIONPOINT_H

#include "frame.h"
#include "../common/aabb.h"
#include "../common/orderedlist.h"
#include <cstdint>
#include <memory>

namespace UnTech {
namespace SpriteImporter {

class ActionPoint {
public:
    typedef OrderedList<Frame, ActionPoint> list_t;

    typedef uint8_t parameter_t;

public:
    ActionPoint() = delete;
    ActionPoint(const ActionPoint&) = delete;

    ActionPoint(Frame& frame);
    ActionPoint(const ActionPoint& point, Frame& frame);

    inline Frame& frame() const { return _frame; }
    inline SpriteImporterDocument& document() const { return _frame.frameSet().document(); }

    upoint location() const { return _location; }
    parameter_t parameter() const { return _parameter; }

    void setLocation(const upoint& location);
    void setParameter(parameter_t parameter);

private:
    Frame& _frame;
    upoint _location;
    parameter_t _parameter;
};
}
}

#endif
