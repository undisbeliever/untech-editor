#ifndef _UNTECH_MODELS_METASPRITE_PALETTE_H
#define _UNTECH_MODELS_METASPRITE_PALETTE_H

#include "frameset.h"
#include "../common/orderedlist.h"
#include "../snes/palette.h"

namespace UnTech {
namespace MetaSprite {

class Palette : public UnTech::Snes::Palette {
public:
    typedef OrderedList<FrameSet, Palette> list_t;

public:
    Palette() = delete;
    Palette(const Palette&) = delete;

    Palette(std::shared_ptr<FrameSet> frameSet)
        : UnTech::Snes::Palette()
        , _frameSet(frameSet)
    {
    }

    Palette(const Palette& p, std::shared_ptr<FrameSet> frameSet)
        : UnTech::Snes::Palette(p)
        : _frameSet(frameSet)
    {
    }

    std::shared_ptr<Palette> clone(std::shared_ptr<FrameSet> frameSet)
    {
        return std::make_shared<Palette>(*this, frameSet);
    }

    inline std::shared_ptr<FrameSet> frameSet() const { return _frameSet.lock(); }
    inline MetaSpriteDocument& document() const { return frameSet()->document(); }

private:
    std::weak_ptr<FrameSet> _frameSet;
};
}
}

#endif
