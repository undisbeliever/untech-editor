#ifndef _UNTECH_MODELS_METASPRITE_FRAMESET_H
#define _UNTECH_MODELS_METASPRITE_FRAMESET_H

#include "../common/ms8aabb.h"
#include "../common/namedlist.h"
#include "../common/orderedlist.h"
#include "../snes/tileset.h"
#include <list>
#include <array>
#include <memory>
#include <string>

namespace UnTech {
namespace MetaSprite {

class Frame;
class MetaSpriteDocument;
class Palette;

class FrameSet : public std::enable_shared_from_this<FrameSet> {

public:
    FrameSet() = delete;
    FrameSet(const FrameSet&) = delete;

    FrameSet(MetaSpriteDocument& document);

    std::shared_ptr<FrameSet> ptr() { return shared_from_this(); }

    inline const std::string& name() const { return _name; }

    inline auto& smallTileset() { return _smallTileset; }
    inline const auto& smallTileset() const { return _smallTileset; }

    inline auto& largeTileset() { return _largeTileset; }
    inline const auto& largeTileset() const { return _largeTileset; }

    inline auto& palettes() { return _palettes; }
    inline const auto& palettes() const { return _palettes; }

    inline auto& frames() { return _frames; }
    inline const auto& frames() const { return _frames; }

    inline MetaSpriteDocument& document() const { return _document; }

    void setName(const std::string& name);

private:
    MetaSpriteDocument& _document;

    std::string _name;

    Snes::Tileset4bpp8px _smallTileset;
    Snes::Tileset4bpp16px _largeTileset;
    OrderedList<FrameSet, Palette> _palettes;

    NamedList<FrameSet, Frame> _frames;
};
}
}

#endif
