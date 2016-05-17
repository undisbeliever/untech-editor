#ifndef _UNTECH_MODELS_METASPRITE_PALETTE_H
#define _UNTECH_MODELS_METASPRITE_PALETTE_H

#include "frameset.h"
#include "../common/orderedlist.h"
#include "../snes/palette.h"

namespace UnTech {
namespace MetaSprite {

class Palette : public UnTech::Snes::Palette4bpp {
public:
    typedef OrderedList<FrameSet, Palette> list_t;

public:
    Palette() = delete;
    Palette(const Palette&) = delete;

    Palette(FrameSet& frameSet)
        : UnTech::Snes::Palette4bpp()
        , _frameSet(frameSet)
    {
    }

    Palette(const Palette& p, FrameSet& frameSet)
        : UnTech::Snes::Palette4bpp(p)
        , _frameSet(frameSet)
    {
    }

    std::shared_ptr<Palette> clone(FrameSet& frameSet)
    {
        return std::make_shared<Palette>(*this, frameSet);
    }

    inline FrameSet& frameSet() const { return _frameSet; }
    inline MetaSpriteDocument& document() const { return _frameSet.document(); }

private:
    FrameSet& _frameSet;
};
}
}

#endif
