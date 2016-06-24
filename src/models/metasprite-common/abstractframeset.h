#pragma once

#include "abstractframeset.h"
#include "animation.h"
#include "tilesettype.h"
#include "models/common/namedlist.h"
#include "models/document.h"
#include <memory>
#include <string>

namespace UnTech {
namespace MetaSpriteCommon {

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
    inline const TilesetType tilesetType() const { return _tilesetType; }

    inline auto& exportOrderDocument() const { return _exportOrderDocument; }
    const std::string& exportOrderFilename() const;

    inline auto& animations() { return _animations; }
    inline const auto& animations() const { return _animations; }

    void setName(const std::string& name);

    void setTilesetType(const TilesetType& type) { _tilesetType = type; }

    // This may cause an exception if there is an error loading filename.
    void loadExportOrderDocument(const std::string& filename);

    virtual bool containsFrameName(const std::string&) const = 0;

private:
    ::UnTech::Document& _document;
    std::string _name;
    TilesetType _tilesetType;
    std::shared_ptr<const FrameSetExportOrder::ExportOrderDocument> _exportOrderDocument;
    NamedList<AbstractFrameSet, Animation> _animations;
};
}
}
