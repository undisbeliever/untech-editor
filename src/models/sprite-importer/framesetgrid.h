#ifndef _UNTECH_MODELS_SPRITEIMPORTER_FRAMESETGRID_H
#define _UNTECH_MODELS_SPRITEIMPORTER_FRAMESETGRID_H

#include "../common/aabb.h"
#include <memory>

namespace UnTech {
namespace SpriteImporter {

class FrameSet;

class FrameSetGrid {
    friend class FrameSet;

public:
    const static unsigned DEFAULT_WIDTH = 32;
    const static unsigned DEFAULT_HEIGHT = 32;

protected:
    FrameSetGrid() = delete;
    FrameSetGrid(const FrameSetGrid&) = delete;

    FrameSetGrid(FrameSet& frameSet);

    void copyFrom(const FrameSetGrid& grid);

public:
    usize frameSize() const { return _frameSize; }
    upoint offset() const { return _offset; }
    usize padding() const { return _padding; }
    upoint origin() const { return _origin; }

    void setFrameSize(usize frameSize);
    void setOffset(upoint offset);
    void setPadding(usize padding);

    void setOrigin(upoint origin);

private:
    FrameSet& _frameSet;

    usize _frameSize;
    upoint _offset;
    usize _padding;
    upoint _origin;
};
}
}

#endif
