#ifndef _UNTECH_MODELS_METASPRITEFORMAT_MSEXPORTORDER_H
#define _UNTECH_MODELS_METASPRITEFORMAT_MSEXPORTORDER_H

// This is a quick job. I may expand on it later.

#include "models/metasprite.h"
#include "models/document.h"
#include <vector>
#include <memory>

namespace UnTech {
namespace MetaSpriteCompiler {

class MsExportOrderDocument;

class MsExportOrder {
public:
    MsExportOrder() = delete;
    MsExportOrder(const MsExportOrder&) = delete;

    MsExportOrder(MsExportOrderDocument& document)
        : _document(document)
        , _frameSets()
    {
    }

    ~MsExportOrder() = default;

    inline MsExportOrderDocument& document() const { return _document; }

    auto& frameSets() { return _frameSets; }
    const auto& frameSets() const { return _frameSets; }

private:
    MsExportOrderDocument& _document;
    std::vector<std::shared_ptr<MetaSprite::MetaSpriteDocument>> _frameSets;
};

// ::MAYDO combine with other project types?::

class MsExportOrderDocument : public ::UnTech::Document {
public:
    MsExportOrderDocument();
    explicit MsExportOrderDocument(const std::string& filename);

    virtual ~MsExportOrderDocument() = default;

    inline auto& exportOrder() { return _msExportOrder; }
    inline const auto& exportOrder() const { return _msExportOrder; }

    virtual void writeDataFile(const std::string& filename) override;

private:
    MsExportOrder _msExportOrder;
};
}
}

#endif
