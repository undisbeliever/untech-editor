#include "frameset.h"
#include "models/metasprite-common/limits.h"
#include "models/metasprite.h"

using namespace UnTech::MetaSprite;

FrameSet::FrameSet(MetaSpriteDocument& document)
    : MetaSpriteCommon::AbstractFrameSet((::UnTech::Document&)(document))
    , _document(document)
    , _smallTileset()
    , _largeTileset()
    , _palettes(*this, MetaSpriteCommon::MAX_PALETTES)
    , _frames(*this, MetaSpriteCommon::MAX_FRAMES)
{
}

bool FrameSet::containsFrameName(const std::string& name) const
{
    return _frames.nameExists(name);
}
