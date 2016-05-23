#pragma once

#include "tilesettype.h"
#include "models/document.h"
#include <memory>
#include <string>

namespace UnTech {
namespace MetaSpriteFormat {

namespace FrameSetExportOrder {
class ExportOrderDocument;
}

class AbstractFrameSet {
public:
    AbstractFrameSet() = delete;
    AbstractFrameSet(const AbstractFrameSet&) = delete;

    AbstractFrameSet(::UnTech::Document& document);

    virtual ~AbstractFrameSet() = default;

    inline ::UnTech::Document& document() const { return _document; }

    inline const std::string& name() const { return _name; }
    inline const MetaSpriteFormat::TilesetType tilesetType() const { return _tilesetType; }

    inline auto& exportOrderDocument() const { return _exportOrderDocument; }
    const std::string& exportOrderFilename() const;

    void setName(const std::string& name);

    void setTilesetType(const MetaSpriteFormat::TilesetType& type) { _tilesetType = type; }

    // This may cause an exception if there is an error loading filename.
    void loadExportOrderDocument(const std::string& filename);

private:
    ::UnTech::Document& _document;
    std::string _name;
    MetaSpriteFormat::TilesetType _tilesetType;
    std::shared_ptr<const FrameSetExportOrder::ExportOrderDocument> _exportOrderDocument;
};
}
}
