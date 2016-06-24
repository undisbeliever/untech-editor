#include "abstractframeset.h"
#include "animation.h"
#include "framesetexportorder.h"
#include <iostream>

using namespace UnTech::MetaSpriteCommon;

AbstractFrameSet::AbstractFrameSet(::UnTech::Document& document)
    : _document(document)
    , _name("frameset")
    , _tilesetType()
    , _exportOrderDocument()
    , _animations(*this)
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
        catch (const std::exception&) {
            _exportOrderDocument = nullptr;
            throw;
        }
    }
    else {
        _exportOrderDocument = nullptr;
    }
}
