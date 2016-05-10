#include "frameset.h"
#include "frame.h"
#include "actionpoint.h"
#include "entityhitbox.h"
#include "frameobject.h"
#include "../common/file.h"
#include "../common/namechecks.h"

using namespace UnTech::SpriteImporter;
namespace MSF = UnTech::MetaSpriteFormat;

FrameSet::FrameSet(SpriteImporterDocument& document)
    : _document(document)
    , _name("frameset")
    , _tilesetType(MSF::TilesetType::Enum::ONE_VRAM_ROW)
    , _imageFilename()
    , _image()
    , _transparentColor(0)
    , _frames(*this)
    , _grid(*this)
{
}

void FrameSet::setName(const std::string& name)
{
    if (isNameValid(name)) {
        _name = name;
    }
}

void FrameSet::setTilesetType(const MSF::TilesetType& type)
{
    _tilesetType = type;
}

void FrameSet::setImageFilename(const std::string& filename)
{
    if (_imageFilename != filename) {
        std::string fpath = File::fullPath(filename);

        _imageFilename = fpath;
        _image.erase();
    }
}

bool FrameSet::reloadImage()
{
    if (!_imageFilename.empty()) {
        auto ret = _image.loadPngImage(_imageFilename);

        if (ret && !transparentColorValid()) {
            _transparentColor = _image.getPixel(0, 0);
        }

        return ret;
    }
    else {
        return false;
    }
}
