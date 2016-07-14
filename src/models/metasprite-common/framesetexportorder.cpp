#include "framesetexportorder.h"
#include "framesetexportorderserializer.h"
#include "models/common/file.h"
#include <unordered_map>

using namespace UnTech;
using namespace UnTech::MetaSpriteCommon::FrameSetExportOrder;

const UnTech::DocumentType ExportOrderDocument::DOCUMENT_TYPE = {
    "UnTech FrameSet Type Document",
    "utft"
};

ExportOrderDocument::ExportOrderDocument()
    : ::UnTech::Document()
    , _exportOrder(*this)
{
}

ExportOrderDocument::ExportOrderDocument(const std::string& filename)
    : ::UnTech::Document(filename)
    , _exportOrder(*this)
{
    readFile(_exportOrder, filename);
}

void ExportOrderDocument::writeDataFile(const std::string& filename)
{
    writeFile(_exportOrder, filename);
    setFilename(filename);
}

std::shared_ptr<const ExportOrderDocument> ExportOrderDocument::loadReadOnly(const std::string& filename)
{
    static std::unordered_map<std::string, std::weak_ptr<const ExportOrderDocument>> openDocuments;

    const std::string fullPath = File::fullPath(filename);

    const auto match = openDocuments.find(fullPath);

    if (match != openDocuments.end()) {
        auto ret = match->second.lock();

        if (ret) {
            return ret;
        }
    }

    auto ret = std::make_shared<const ExportOrderDocument>(fullPath);

    openDocuments[fullPath] = ret;

    return ret;
}

const DocumentType& ExportOrderDocument::documentType() const
{
    return DOCUMENT_TYPE;
}
