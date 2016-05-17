#ifndef _UNTECH_MODELS_SPRITEIMPORTER_FRAME_H
#define _UNTECH_MODELS_SPRITEIMPORTER_FRAME_H

#include "frameset.h"
#include "../common/aabb.h"
#include "../common/namedlist.h"
#include "../common/orderedlist.h"
#include <memory>

namespace UnTech {
namespace SpriteImporter {

class FrameObject;
class ActionPoint;
class EntityHitbox;

class Frame {
public:
    const static unsigned MIN_WIDTH = 16;
    const static unsigned MIN_HEIGHT = 16;
    const static usize MIN_SIZE;

    const static unsigned SPRITE_ORDER_MASK = 3;
    const static unsigned DEFAULT_SPRITE_ORDER = 2;

    typedef NamedList<FrameSet, Frame> list_t;

public:
    Frame() = delete;
    Frame(const Frame&) = delete;

    Frame(FrameSet& frameSet);
    Frame(const Frame& frame, FrameSet& frameSet);

    inline FrameSet& frameSet() const { return _frameSet; }
    inline SpriteImporterDocument& document() const { return _frameSet.document(); }

    inline auto& objects() { return _objects; }
    inline auto& actionPoints() { return _actionPoints; }
    inline auto& entityHitboxes() { return _entityHitboxes; }

    inline const auto& objects() const { return _objects; }
    inline const auto& actionPoints() const { return _actionPoints; }
    inline const auto& entityHitboxes() const { return _entityHitboxes; }

    bool useGridLocation() const { return _useGridLocation; }
    upoint gridLocation() const { return _gridLocation; }
    urect location() const { return _location; }
    usize locationSize() const { return _location.size(); }

    bool useGridOrigin() const { return _useGridOrigin; }
    upoint origin() const { return _origin; }

    bool solid() const { return _solid; }
    urect tileHitbox() const { return _tileHitbox; }

    unsigned spriteOrder() const { return _spriteOrder; }

    void setUseGridLocation(bool useGridLocation);
    void setGridLocation(upoint gridLocation);
    void setLocation(const urect& location);

    void setUseGridOrigin(bool useGridOrigin);
    void setOrigin(upoint origin);

    void setSolid(bool solid);
    void setTileHitbox(const urect& tileHitbox);

    void setSpriteOrder(unsigned spriteOrder);

    /**
     * Calculates the minimum allowable size of the frame.
     */
    usize minimumViableSize() const;

    /**
     * Recalculates the frame location (if useGridLocation is set).
     */
    void recalculateLocation();

    /**
     * Recalculates the frame origin (if useGridOrigin is set).
     */
    void recalculateOrigin();

private:
    FrameSet& _frameSet;
    OrderedList<Frame, FrameObject> _objects;
    OrderedList<Frame, ActionPoint> _actionPoints;
    OrderedList<Frame, EntityHitbox> _entityHitboxes;

    bool _useGridLocation;
    upoint _gridLocation;
    urect _location;

    bool _useGridOrigin;
    upoint _origin;

    bool _solid;
    urect _tileHitbox;

    unsigned _spriteOrder;
};
}
}

#endif
