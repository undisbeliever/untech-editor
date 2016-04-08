#ifndef _UNTECH_MODELS_SPRITEIMPORTER_FRAMEOBJECT_H
#define _UNTECH_MODELS_SPRITEIMPORTER_FRAMEOBJECT_H

#include "frame.h"
#include "../common/aabb.h"
#include "../common/orderedlist.h"
#include <memory>

namespace UnTech {
namespace SpriteImporter {

class FrameObject {
public:
    typedef OrderedList<Frame, FrameObject> list_t;

    enum class ObjectSize {
        SMALL = 8,
        LARGE = 16
    };

public:
    FrameObject() = delete;
    FrameObject(const FrameObject&) = delete;

    FrameObject(std::shared_ptr<Frame> frame);
    FrameObject(const FrameObject& object, std::shared_ptr<Frame> frame);

    std::shared_ptr<FrameObject> clone(std::shared_ptr<Frame> frame);

    upoint location() const { return _location; }
    ObjectSize size() const { return _size; }

    unsigned sizePx() const { return (unsigned)_size; }

    inline std::shared_ptr<Frame> frame() const { return _frame.lock(); }
    inline SpriteImporterDocument& document() const { return frame()->frameSet()->document(); }

    void setLocation(const upoint& location);
    void setSize(ObjectSize size);

    /**
     * bottom left point of the frame.
     */
    upoint bottomLeft() const
    {
        return upoint(_location.x + sizePx(), _location.y + sizePx());
    }

protected:
    void validateLocation();

private:
    std::weak_ptr<Frame> _frame;
    upoint _location;
    ObjectSize _size;
};
}
}

#endif
