#include "frameset.h"
#include "document.h"
#include "frame.h"
#include "actionpoint.h"
#include "frameobject.h"
#include "entityhitbox.h"
#include "palette.h"
#include "../common/file.h"
#include "../common/namechecks.h"

using namespace UnTech::MetaSprite;
namespace MSF = UnTech::MetaSpriteFormat;

FrameSet::FrameSet(MetaSpriteDocument& document)
    : _document(document)
    , _name("frameset")
    , _tilesetType()
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
