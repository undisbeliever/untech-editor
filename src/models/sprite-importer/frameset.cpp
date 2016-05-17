#include "frameset.h"
#include "actionpoint.h"
#include "document.h"
#include "entityhitbox.h"
#include "frame.h"
#include "frameobject.h"
#include "models/common/file.h"
#include "models/common/namechecks.h"

using namespace UnTech::SpriteImporter;
namespace MSF = UnTech::MetaSpriteFormat;
namespace FSExportOrder = UnTech::MetaSpriteFormat::FrameSetExportOrder;

FrameSet::FrameSet(SpriteImporterDocument& document)
    : MetaSpriteFormat::AbstractFrameSet(document)
    , _document(document)
    , _imageFilename()
    , _image()
    , _transparentColor(0)
    , _frames(*this)
    , _grid(*this)
{
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
