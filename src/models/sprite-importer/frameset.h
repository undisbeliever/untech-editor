#ifndef _UNTECH_MODELS_SPRITEIMPORTER_FRAMESET_H
#define _UNTECH_MODELS_SPRITEIMPORTER_FRAMESET_H

#include "framesetgrid.h"
#include "../common/aabb.h"
#include "../common/namedlist.h"
#include <memory>
#include <string>

namespace UnTech {
namespace SpriteImporter {

class Frame;

class FrameSet : public std::enable_shared_from_this<FrameSet> {

public:
    typedef NamedList<FrameSet> list_t;

public:
    FrameSet();

    std::shared_ptr<FrameSet> ptr() { return shared_from_this(); }
    std::shared_ptr<FrameSet> clone();

    inline auto imageFilename() const { return _imageFilename; }

    inline auto& frames() { return _frames; }
    inline auto& grid() { return _grid; }

    inline const auto& frames() const { return _frames; }
    inline const auto& grid() const { return _grid; }

    bool setImageFilename(const std::string& filename);

private:
    // ::TODO parent::

    std::string _imageFilename;
    // ::TODO image::

    NamedList<FrameSet, Frame> _frames;
    FrameSetGrid _grid;
};
}
}

#endif
