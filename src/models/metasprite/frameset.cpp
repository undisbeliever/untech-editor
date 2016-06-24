#include "frameset.h"
#include "models/metasprite.h"

using namespace UnTech::MetaSprite;

FrameSet::FrameSet(MetaSpriteDocument& document)
    : MetaSpriteCommon::AbstractFrameSet((::UnTech::Document&)(document))
    , _document(document)
    , _smallTileset()
    , _largeTileset()
    , _palettes(*this)
    , _frames(*this)
{
}

bool FrameSet::containsFrameName(const std::string& name) const
{
    return _frames.nameExists(name);
}
