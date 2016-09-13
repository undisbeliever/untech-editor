#include "frameset.h"
#include "actionpoint.h"
#include "document.h"
#include "entityhitbox.h"
#include "frame.h"
#include "frameobject.h"
#include "models/common/file.h"
#include "models/common/namechecks.h"
#include "models/metasprite-common/limits.h"

using namespace UnTech::SpriteImporter;
namespace FSExportOrder = UnTech::MetaSpriteCommon::FrameSetExportOrder;

const char* FrameSet::TYPE_NAME = "FrameSet";

FrameSet::FrameSet(SpriteImporterDocument& document)
    : MetaSpriteCommon::AbstractFrameSet(document)
    , _document(document)
    , _imageFilename()
    , _image()
    , _transparentColor(rgba::fromRgba(0))
    , _frames(*this, MetaSpriteCommon::MAX_FRAMES)
    , _grid(*this)
{
}

bool FrameSet::containsFrameName(const std::string& name) const
{
    return _frames.nameExists(name);
}

void FrameSet::setImageFilename(const std::string& filename)
{
    if (_imageFilename != filename) {
        std::string fpath = File::fullPath(filename);

        _imageFilename = fpath;
        reloadImage();
    }
}

bool FrameSet::reloadImage()
{
    if (!_imageFilename.empty()) {
        _image.erase();
    }
    auto ret = _image.loadPngImage(_imageFilename);

    if (ret && !transparentColorValid()) {
        _transparentColor = _image.getPixel(0, 0);
    }

    return ret;
}
