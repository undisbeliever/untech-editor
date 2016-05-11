#include "frameset.h"
#include "frame.h"
#include "actionpoint.h"
#include "entityhitbox.h"
#include "frameobject.h"
#include "models/common/file.h"
#include "models/common/namechecks.h"
#include "models/metasprite-format/framesetexportorder.h"
#include <iostream>

using namespace UnTech::SpriteImporter;
namespace MSF = UnTech::MetaSpriteFormat;
namespace FSExportOrder = UnTech::MetaSpriteFormat::FrameSetExportOrder;

FrameSet::FrameSet(SpriteImporterDocument& document)
    : _document(document)
    , _name("frameset")
    , _tilesetType(MSF::TilesetType::Enum::ONE_VRAM_ROW)
    , _exportOrderDocument()
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

void FrameSet::setTilesetType(const MSF::TilesetType& type)
{
    _tilesetType = type;
}

void FrameSet::setImageFilename(const std::string& filename)
{
    if (_imageFilename != filename) {
        std::string fpath = File::fullPath(filename);

        _imageFilename = fpath;
        _image.erase();
    }
}

void FrameSet::loadExportOrderDocument(const std::string& filename)
{
    if (!filename.empty()) {
        try {
            _exportOrderDocument = FSExportOrder::ExportOrderDocument::loadReadOnly(filename);
        }
        catch (std::exception& ex) {
            std::cerr << "Error loading " << filename << ": " << ex.what() << std::endl;
            _exportOrderDocument = nullptr;
        }
    }
    else {
        _exportOrderDocument = nullptr;
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
