#ifndef _UNTECH_MODELS_METASPRITE_FRAME_H
#define _UNTECH_MODELS_METASPRITE_FRAME_H

#include "frameset.h"
#include "../common/ms8aabb.h"
#include "../common/namedlist.h"
#include "../common/orderedlist.h"
#include <memory>

namespace UnTech {
namespace MetaSprite {

class FrameObject;
class ActionPoint;
class EntityHitbox;

class Frame : public std::enable_shared_from_this<Frame> {
public:
    typedef NamedList<FrameSet, Frame> list_t;

public:
    Frame() = delete;
    Frame(const Frame&) = delete;

    Frame(std::shared_ptr<FrameSet> frameSet);

private:
    Frame(const Frame& frame, std::shared_ptr<FrameSet> frameSet);

public:
    std::shared_ptr<Frame> ptr() { return shared_from_this(); }
    std::shared_ptr<Frame> clone(std::shared_ptr<FrameSet> frameSet);

    inline auto& objects() { return _objects; }
    inline auto& actionPoints() { return _actionPoints; }
    inline auto& entityHitboxes() { return _entityHitboxes; }

    inline const auto& objects() const { return _objects; }
    inline const auto& actionPoints() const { return _actionPoints; }
    inline const auto& entityHitboxes() const { return _entityHitboxes; }

    inline bool solid() const { return _solid; }
    inline ms8rect tileHitbox() const { return _tileHitbox; }

    inline std::shared_ptr<FrameSet> frameSet() const { return _frameSet.lock(); }
    inline MetaSpriteDocument& document() const { return frameSet()->document(); }

    inline void setSolid(bool solid) { _solid = solid; }
    inline void setTileHitbox(const ms8rect& tileHitbox) { _tileHitbox = tileHitbox; }

    struct Boundary {
        int x, y;
        unsigned width, height;
    };
    Boundary calcBoundary() const;

private:
    std::weak_ptr<FrameSet> _frameSet;
    OrderedList<Frame, FrameObject> _objects;
    OrderedList<Frame, ActionPoint> _actionPoints;
    OrderedList<Frame, EntityHitbox> _entityHitboxes;

    bool _solid;
    ms8rect _tileHitbox;
};
}
}

#endif
