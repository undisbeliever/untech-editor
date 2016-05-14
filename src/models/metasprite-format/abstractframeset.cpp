#include "abstractframeset.h"
#include "framesetexportorder.h"
#include <iostream>

using namespace UnTech::MetaSpriteFormat;

AbstractFrameSet::AbstractFrameSet(::UnTech::Document& document)
    : _document(document)
    , _name("frameset")
    , _tilesetType()
    , _exportOrderDocument()
{
}

const std::string& AbstractFrameSet::exportOrderFilename() const
{
    const static std::string emptyString;

    if (_exportOrderDocument) {
        return _exportOrderDocument->filename();
    }
    else {
        return emptyString;
    }
}

void AbstractFrameSet::setName(const std::string& name)
{
    if (isNameValid(name)) {
        _name = name;
    }
}

void AbstractFrameSet::loadExportOrderDocument(const std::string& filename)
{
    if (!filename.empty()) {
        try {
            _exportOrderDocument = FrameSetExportOrder::ExportOrderDocument::loadReadOnly(filename);
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
