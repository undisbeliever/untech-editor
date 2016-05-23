#pragma once

#include "framesetgrid.h"
#include "models/common/aabb.h"
#include "models/common/image.h"
#include "models/common/namedlist.h"
#include "models/common/rgba.h"
#include "models/metasprite-format/abstractframeset.h"
#include <memory>
#include <string>

// FrameSet images are lazily loaded.

namespace UnTech {

namespace MetaSpriteFormat {
namespace FrameSetExportOrder {
class ExportOrderDocument;
}
}

namespace SpriteImporter {

class Frame;
class SpriteImporterDocument;

class FrameSet : public MetaSpriteFormat::AbstractFrameSet {

public:
    FrameSet() = delete;
    FrameSet(const FrameSet&) = delete;

    FrameSet(SpriteImporterDocument& document);

    inline SpriteImporterDocument& document() const { return _document; }

    inline auto& frames() { return _frames; }
    inline auto& grid() { return _grid; }
    inline const std::string& imageFilename() const { return _imageFilename; }
    inline auto& transparentColor() const { return _transparentColor; }

    inline const auto& frames() const { return _frames; }
    inline const auto& grid() const { return _grid; }

    // fails silently
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

    std::string _imageFilename;
    UnTech::Image _image;
    UnTech::rgba _transparentColor;

    NamedList<FrameSet, Frame> _frames;
    FrameSetGrid _grid;
};
}
}
