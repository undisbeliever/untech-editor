#include "msexportorder.h"
#include "msexportorderserializer.h"

using namespace UnTech::MetaSpriteCompiler;

const UnTech::DocumentType MsExportOrderDocument::DOCUMENT_TYPE = {
    "UnTech MetaSprite Export Document",
    "utex"
};

MsExportOrderDocument::MsExportOrderDocument()
    : ::UnTech::Document()
    , _msExportOrder(*this)
{
}

MsExportOrderDocument::MsExportOrderDocument(const std::string& filename)
    : ::UnTech::Document(filename)
    , _msExportOrder(*this)
{
    Serializer::readFile(_msExportOrder, filename);
}

void MsExportOrderDocument::writeDataFile(const std::string& filename)
{
    Serializer::writeFile(_msExportOrder, filename);
    setFilename(filename);
}

const UnTech::DocumentType& MsExportOrderDocument::documentType() const
{
    return DOCUMENT_TYPE;
}
