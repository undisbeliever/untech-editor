#ifndef _UNTECH_MODELS_METASPRITE_FRAMESET_H
#define _UNTECH_MODELS_METASPRITE_FRAMESET_H

#include "../common/ms8aabb.h"
#include "../common/namedlist.h"
#include <memory>
#include <string>

namespace UnTech {
namespace MetaSprite {

class Frame;
class MetaSpriteDocument;

class FrameSet : public std::enable_shared_from_this<FrameSet> {

public:
    FrameSet() = delete;
    FrameSet(const FrameSet&) = delete;

    FrameSet(MetaSpriteDocument& document);

    std::shared_ptr<FrameSet> ptr() { return shared_from_this(); }

    inline const std::string& name() const { return _name; }

    inline auto& frames() { return _frames; }
    inline const auto& frames() const { return _frames; }

    inline MetaSpriteDocument& document() const { return _document; }

    void setName(const std::string& name);

private:
    MetaSpriteDocument& _document;

    std::string _name;

    NamedList<FrameSet, Frame> _frames;

    // ::TODO palettes::
    // ::TODO tilesets::
};
}
}

#endif
