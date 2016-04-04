#include "frameset.h"
#include "frame.h"
#include "../common/file.h"
#include "../common/namechecks.h"

using namespace UnTech::SpriteImporter;

FrameSet::FrameSet(SpriteImporterDocument& document)
    : _document(document)
    , _name()
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
