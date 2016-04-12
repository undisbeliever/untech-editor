#include "frameset.h"
#include "document.h"
#include "frame.h"
#include "../common/file.h"
#include "../common/namechecks.h"

using namespace UnTech::MetaSprite;

FrameSet::FrameSet(MetaSpriteDocument& document)
    : _document(document)
    , _name("frameset")
    , _smallTileset()
    , _largeTileset()
    , _palettes(*this)
    , _frames(*this)
{
}

void FrameSet::setName(const std::string& name)
{
    if (isNameValid(name)) {
        _name = name;
    }
}
