#ifndef _UNTECH_MODELS_SPRITEIMPORTER_FRAMESET_H
#define _UNTECH_MODELS_SPRITEIMPORTER_FRAMESET_H

#include "framesetgrid.h"
#include "../common/aabb.h"
#include "../common/rgba.h"
#include "../common/image.h"
#include "../common/namedlist.h"
#include "../common/namedlistref.h"
#include "../document.h"
#include <memory>
#include <string>

// FrameSet images are lazily loaded.

namespace UnTech {

class Document;

namespace SpriteImporter {

class Frame;

class FrameSet : public std::enable_shared_from_this<FrameSet> {

public:
    typedef NamedListRef<Document, FrameSet> list_t;

public:
    FrameSet(Document& _document);

    std::shared_ptr<FrameSet> ptr() { return shared_from_this(); }
    std::shared_ptr<FrameSet> clone(Document& _document);

    inline auto imageFilename() const { return _imageFilename; }

    inline auto& frames() { return _frames; }
    inline auto& grid() { return _grid; }
    inline auto& transparentColor() const { return _transparentColor; }

    inline const auto& frames() const { return _frames; }
    inline const auto& grid() const { return _grid; }

    inline Document& document() const { return _document; }

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
    Document& _document;

    std::string _imageFilename;
    UnTech::Image _image;
    UnTech::rgba _transparentColor;

    NamedList<FrameSet, Frame> _frames;
    FrameSetGrid _grid;
};
}
}

#endif
