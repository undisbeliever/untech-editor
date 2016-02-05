#include "frameset.h"

using namespace UnTech::SpriteImporter;

FrameSet::FrameSet()
    : _imageFilename()
    , _frames(*this)
    , _grid(*this)
{
}

bool FrameSet::setImageFilename(const std::string& filename)
{
    if (_imageFilename != filename) {
        // ::TODO load image::
        _imageFilename = filename;
    }

    return true;
}
