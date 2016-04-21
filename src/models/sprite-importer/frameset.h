#ifndef _UNTECH_MODELS_SPRITEIMPORTER_FRAMESET_H
#define _UNTECH_MODELS_SPRITEIMPORTER_FRAMESET_H

#include "framesetgrid.h"
#include "../common/aabb.h"
#include "../common/rgba.h"
#include "../common/image.h"
#include "../common/namedlist.h"
#include <memory>
#include <string>

// FrameSet images are lazily loaded.

namespace UnTech {
namespace SpriteImporter {

class Frame;
class SpriteImporterDocument;

class FrameSet {

public:
    FrameSet() = delete;
    FrameSet(const FrameSet&) = delete;

    FrameSet(SpriteImporterDocument& document);

    inline SpriteImporterDocument& document() const { return _document; }

    inline const std::string& name() const { return _name; }
    inline const std::string& imageFilename() const { return _imageFilename; }

    inline auto& frames() { return _frames; }
    inline auto& grid() { return _grid; }
    inline auto& transparentColor() const { return _transparentColor; }

    inline const auto& frames() const { return _frames; }
    inline const auto& grid() const { return _grid; }

    void setName(const std::string& name);

    void setImageFilename(const std::string& filename);

    void setTransparentColor(const UnTech::rgba transparentColor)
    {
        _transparentColor = transparentColor;
    }

    bool transparentColorValid() const
    {
        return _transparentColor.value != 0 && _transparentColor.alpha == 0xFF;
    }

    inline UnTech::Image& image()
    {
        if (_image.empty() && !_imageFilename.empty()) {
            reloadImage();
        }
        return _image;
    }

    bool reloadImage();

private:
    SpriteImporterDocument& _document;

    std::string _name;
    std::string _imageFilename;
    UnTech::Image _image;
    UnTech::rgba _transparentColor;

    NamedList<FrameSet, Frame> _frames;
    FrameSetGrid _grid;
};
}
}

#endif
