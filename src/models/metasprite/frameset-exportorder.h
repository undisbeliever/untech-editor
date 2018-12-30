/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "errorlist.h"
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

        bool frameExists(const MetaSprite::Frame::map_t& frameMap) const;
        bool frameExists(const SpriteImporter::Frame::map_t& frameMap) const;
        bool animationExists(const Animation::Animation::map_t& animationMap) const;
    };

    idstring name;
    std::vector<ExportName> stillFrames;
    std::vector<ExportName> animations;

    FrameSetExportOrder() = default;
    FrameSetExportOrder(const FrameSetExportOrder&) = delete;

    bool validate(Resources::ErrorList& err) const;

    bool testFrameSet(const MetaSprite::FrameSet& frameSet, ErrorList& errorList) const;
    bool testFrameSet(const SpriteImporter::FrameSet& frameSet, ErrorList& errorList) const;
};

// throws exception on error
std::unique_ptr<FrameSetExportOrder> loadFrameSetExportOrder(const std::string& filename);
void saveFrameSetExportOrder(const FrameSetExportOrder& eo, const std::string& filename);
}
}
