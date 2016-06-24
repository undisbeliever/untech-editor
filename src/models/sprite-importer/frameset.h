#pragma once

#include "framesetgrid.h"
#include "models/common/aabb.h"
#include "models/common/image.h"
#include "models/common/namedlist.h"
#include "models/common/rgba.h"
#include "models/metasprite-common/abstractframeset.h"
#include <memory>
#include <string>

namespace UnTech {

namespace MetaSpriteCommon {
namespace FrameSetExportOrder {
class ExportOrderDocument;
}
}

namespace SpriteImporter {

class Frame;
class SpriteImporterDocument;

class FrameSet : public MetaSpriteCommon::AbstractFrameSet {
public:
    static const char* TYPE_NAME;

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

    void setImageFilename(const std::string& filename);

    void setTransparentColor(const UnTech::rgba transparentColor)
    {
        _transparentColor = transparentColor;
    }

    bool transparentColorValid() const
    {
        return _transparentColor.value != 0 && _transparentColor.alpha == 0xFF;
    }

    inline UnTech::Image& image() { return _image; }
    inline const UnTech::Image& image() const { return _image; }

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
