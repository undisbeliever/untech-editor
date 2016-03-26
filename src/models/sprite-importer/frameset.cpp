#include "frameset.h"
#include "frame.h"

using namespace UnTech::SpriteImporter;

FrameSet::FrameSet(Document& document)
    : _document(document)
    , _imageFilename()
    , _image()
    , _transparentColor(0)
    , _frames(*this)
    , _grid(*this)
{
}

std::shared_ptr<FrameSet> FrameSet::clone(Document& document)
{
    auto fs = std::make_shared<FrameSet>(document);

    fs->setImageFilename(this->_imageFilename);
    fs->_grid.copyFrom(this->_grid);

    for (const auto& f : _frames) {
        fs->_frames.clone(f.second, f.first);
    }

    return fs;
}

void FrameSet::setImageFilename(const std::string& filename)
{
    if (_imageFilename != filename) {
        _imageFilename = filename;
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
