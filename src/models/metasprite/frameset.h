#pragma once

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
    static const char* TYPE_NAME;

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

    template <class TilesetT>
    inline TilesetT& getTileset();

    template <class TilesetT>
    inline const TilesetT& getTileset() const;

private:
    MetaSpriteDocument& _document;

    Snes::Tileset4bpp8px _smallTileset;
    Snes::Tileset4bpp16px _largeTileset;
    OrderedList<FrameSet, Palette> _palettes;

    NamedList<FrameSet, Frame> _frames;
};

template <>
inline Snes::Tileset4bpp8px& FrameSet::getTileset<Snes::Tileset4bpp8px>()
{
    return _smallTileset;
}

template <>
inline Snes::Tileset4bpp16px& FrameSet::getTileset<Snes::Tileset4bpp16px>()
{
    return _largeTileset;
}

template <>
inline const Snes::Tileset4bpp8px& FrameSet::getTileset<Snes::Tileset4bpp8px>() const
{
    return _smallTileset;
}

template <>
inline const Snes::Tileset4bpp16px& FrameSet::getTileset<Snes::Tileset4bpp16px>() const
{
    return _largeTileset;
}
}
}
