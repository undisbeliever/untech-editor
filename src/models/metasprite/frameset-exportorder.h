/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "models/common/capped_vector.h"
#include "models/common/idstring.h"
#include <memory>
#include <string>
#include <vector>

namespace UnTech {
namespace MetaSprite {

struct FrameSetExportOrder {
    static const std::string FILE_EXTENSION;

    struct ExportName {
        typedef capped_vector<ExportName, MAX_EXPORT_NAMES> list_t;

        idstring name;
        std::vector<NameReference> alternatives;
    };

    std::string filename;

    idstring name;
    ExportName::list_t stillFrames;
    ExportName::list_t animations;

    FrameSetExportOrder() = default;
    FrameSetExportOrder(const FrameSetExportOrder&) = delete;
};

// uses a cache to return the same FrameSetExportOrder for the same filename.
std::shared_ptr<const FrameSetExportOrder>
loadFrameSetExportOrderCached(const std::string& filename);
}
}
