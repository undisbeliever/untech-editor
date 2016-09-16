#pragma once

#include "common.h"
#include "models/common/capped_vector.h"
#include <memory>
#include <string>
#include <vector>

namespace UnTech {
namespace MetaSprite {

struct FrameSetExportOrder {
    static const std::string FILE_EXTENSION;

    struct ExportName {
        typedef capped_vector<ExportName, MAX_EXPORT_NAMES> list_t;

        std::string name;
        std::vector<NameReference> alternatives;
    };

    std::string filename;

    // ::TODO replace with idstring::
    std::string name;
    ExportName::list_t stillFrames;
    ExportName::list_t animations;

    FrameSetExportOrder() = default;
    FrameSetExportOrder(const FrameSetExportOrder&) = delete;
};

// FrameSetExportOrder is read only and
// may be shared amongst many MetaSprite/SpriteImporter instances.
std::shared_ptr<const FrameSetExportOrder>
loadFrameSetExportOrder(const std::string& filename);
}
}
