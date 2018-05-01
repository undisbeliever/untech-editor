/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "models/common/idstring.h"
#include <memory>
#include <string>
#include <vector>

namespace UnTech {
namespace Resources {
class ErrorList;
}
namespace MetaSprite {

struct FrameSetExportOrder {
    static const std::string FILE_EXTENSION;

    struct ExportName {
        idstring name;
        std::vector<NameReference> alternatives;
    };

    idstring name;
    std::vector<ExportName> stillFrames;
    std::vector<ExportName> animations;

    FrameSetExportOrder() = default;
    FrameSetExportOrder(const FrameSetExportOrder&) = delete;

    bool validate(Resources::ErrorList& err) const;
};

// throws exception on error
std::unique_ptr<FrameSetExportOrder> loadFrameSetExportOrder(const std::string& filename);
void saveFrameSetExportOrder(const FrameSetExportOrder& eo, const std::string& filename);
}
}
