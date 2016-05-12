#include "frameset.h"
#include "document.h"
#include "frame.h"
#include "actionpoint.h"
#include "frameobject.h"
#include "entityhitbox.h"
#include "palette.h"
#include "models/common/file.h"
#include "models/common/namechecks.h"
#include "models/metasprite-format/framesetexportorder.h"
#include <iostream>

using namespace UnTech::MetaSprite;
namespace FSExportOrder = UnTech::MetaSpriteFormat::FrameSetExportOrder;

FrameSet::FrameSet(MetaSpriteDocument& document)
    : _document(document)
    , _name("frameset")
    , _tilesetType()
    , _exportOrderDocument()
    , _smallTileset()
    , _largeTileset()
    , _palettes(*this)
    , _frames(*this)
{
}

const std::string& FrameSet::exportOrderFilename() const
{
    const static std::string emptyString;

    if (_exportOrderDocument) {
        return _exportOrderDocument->filename();
    }
    else {
        return emptyString;
    }
}

void FrameSet::setName(const std::string& name)
{
    if (isNameValid(name)) {
        _name = name;
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
