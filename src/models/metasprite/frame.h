#ifndef _UNTECH_MODELS_METASPRITE_FRAME_H
#define _UNTECH_MODELS_METASPRITE_FRAME_H

#include "frameset.h"
#include "../common/image.h"
#include "../common/ms8aabb.h"
#include "../common/namedlist.h"
#include "../common/orderedlist.h"
#include <memory>

namespace UnTech {
namespace MetaSprite {

class FrameObject;
class ActionPoint;
class EntityHitbox;

class Frame {
public:
    typedef NamedList<FrameSet, Frame> list_t;

public:
    Frame() = delete;
    Frame(const Frame&) = delete;

    Frame(FrameSet& frameSet);
    Frame(const Frame& frame, FrameSet& frameSet);

    inline FrameSet& frameSet() const { return _frameSet; }
    inline MetaSpriteDocument& document() const { return _frameSet.document(); }

    inline auto& objects() { return _objects; }
    inline auto& actionPoints() { return _actionPoints; }
    inline auto& entityHitboxes() { return _entityHitboxes; }

    inline const auto& objects() const { return _objects; }
    inline const auto& actionPoints() const { return _actionPoints; }
    inline const auto& entityHitboxes() const { return _entityHitboxes; }

    inline bool solid() const { return _solid; }
    inline ms8rect tileHitbox() const { return _tileHitbox; }

    inline void setSolid(bool solid) { _solid = solid; }
    inline void setTileHitbox(const ms8rect& tileHitbox) { _tileHitbox = tileHitbox; }

    struct Boundary {
        int x, y;
        unsigned width, height;
    };
    Boundary calcBoundary() const;

    void draw(Image& image, const Palette& palette,
              unsigned xOffset = 0, unsigned yOffset = 0) const;

    // returns a unique_ptr so it can be easily inserted into a namedlist
    std::unique_ptr<Frame> flip(bool hFlip, bool vFlip) const;

private:
    FrameSet& _frameSet;
    OrderedList<Frame, FrameObject> _objects;
    OrderedList<Frame, ActionPoint> _actionPoints;
    OrderedList<Frame, EntityHitbox> _entityHitboxes;

    bool _solid;
    ms8rect _tileHitbox;
};
}
}

#endif
