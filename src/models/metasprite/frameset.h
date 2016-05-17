#ifndef _UNTECH_MODELS_METASPRITE_FRAMESET_H
#define _UNTECH_MODELS_METASPRITE_FRAMESET_H

#include "models/common/ms8aabb.h"
#include "models/common/namedlist.h"
#include "models/common/orderedlist.h"
#include "models/metasprite-format/abstractframeset.h"
#include "models/snes/tileset.h"
#include <array>
#include <list>
#include <memory>
#include <string>

namespace UnTech {
namespace MetaSprite {

class Frame;
class Palette;
class MetaSpriteDocument;

class FrameSet : public MetaSpriteFormat::AbstractFrameSet {

public:
    FrameSet() = delete;
    FrameSet(const FrameSet&) = delete;

    FrameSet(MetaSpriteDocument& document)
        : MetaSpriteFormat::AbstractFrameSet((::UnTech::Document&)(document))
        , _document(document)
        , _smallTileset()
        , _largeTileset()
        , _palettes(*this)
        , _frames(*this)
    {
    }

    inline MetaSpriteDocument& document() const { return _document; }

    inline auto& smallTileset() { return _smallTileset; }
    inline const auto& smallTileset() const { return _smallTileset; }

    inline auto& largeTileset() { return _largeTileset; }
    inline const auto& largeTileset() const { return _largeTileset; }

    inline auto& palettes() { return _palettes; }
    inline const auto& palettes() const { return _palettes; }

    inline auto& frames() { return _frames; }
    inline const auto& frames() const { return _frames; }

private:
    MetaSpriteDocument& _document;

    Snes::Tileset4bpp8px _smallTileset;
    Snes::Tileset4bpp16px _largeTileset;
    OrderedList<FrameSet, Palette> _palettes;

    NamedList<FrameSet, Frame> _frames;
};
}
}

#endif
