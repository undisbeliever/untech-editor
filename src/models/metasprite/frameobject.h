#ifndef _UNTECH_MODELS_METASPRITE_FRAMEOBJECT_H
#define _UNTECH_MODELS_METASPRITE_FRAMEOBJECT_H

#include "frame.h"
#include "../common/ms8aabb.h"
#include "../common/orderedlist.h"
#include <cstdint>
#include <memory>

namespace UnTech {
namespace MetaSprite {

class FrameObject {
public:
    typedef OrderedList<Frame, FrameObject> list_t;

    enum class ObjectSize {
        SMALL = 8,
        LARGE = 16
    };

    const static unsigned ORDER_MASK = 3;
    const static unsigned DEFAULT_ORDER = 2;

public:
    FrameObject() = delete;
    FrameObject(const FrameObject&) = delete;

    FrameObject(Frame& frame)
        : _frame(frame)
        , _location(-4, -4)
        , _size(ObjectSize::SMALL)
        , _tileId(0)
        , _order(DEFAULT_ORDER)
        , _hFlip(false)
        , _vFlip(false)
    {
    }

    FrameObject(const FrameObject& object, Frame& frame)
        : _frame(frame)
        , _location(object._location)
        , _size(object._size)
        , _tileId(object._tileId)
        , _order(object._order)
        , _hFlip(object._hFlip)
        , _vFlip(object._vFlip)
    {
    }

    inline Frame& frame() const { return _frame; }
    inline MetaSpriteDocument& document() const { return _frame.frameSet().document(); }

    inline ms8point location() const { return _location; }
    inline ObjectSize size() const { return _size; }
    inline unsigned tileId() const { return _tileId; }
    inline uint_fast8_t order() const { return _order; }
    inline bool hFlip() const { return _hFlip; }
    inline bool vFlip() const { return _vFlip; }

    inline unsigned sizePx() const { return (unsigned)_size; }

    inline void setLocation(const ms8point& location) { _location = location; }
    inline void setSize(ObjectSize size) { _size = size; }
    inline void setTileId(unsigned tileId) { _tileId = tileId; }
    inline void setOrder(uint_fast8_t order) { _order = order & ORDER_MASK; }
    inline void setHFlip(bool hFlip) { _hFlip = hFlip; }
    inline void setVFlip(bool vFlip) { _vFlip = vFlip; }

private:
    Frame& _frame;
    ms8point _location;
    ObjectSize _size;
    unsigned _tileId;
    uint_fast8_t _order;
    bool _hFlip;
    bool _vFlip;
};
}
}

#endif
