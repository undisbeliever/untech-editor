#include "frameset.h"
#include "frame.h"

using namespace UnTech::SpriteImporter;

FrameSet::FrameSet()
    : _imageFilename()
    , _frames(*this)
    , _grid(*this)
{
}

std::shared_ptr<FrameSet> FrameSet::clone()
{
    auto fs = std::make_shared<FrameSet>();

    fs->setImageFilename(this->_imageFilename);
    fs->_grid.copyFrom(this->_grid);

    for (const auto& f : _frames) {
        fs->_frames.clone(f.second, f.first);
    }

    return fs;
}

bool FrameSet::setImageFilename(const std::string& filename)
{
    if (_imageFilename != filename) {
        // ::TODO load image::
        _imageFilename = filename;
    }

    return true;
}
