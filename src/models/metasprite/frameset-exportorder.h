/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include "metasprite.h"
#include "spriteimporter.h"
#include "models/common/idstring.h"
#include <memory>
#include <string>
#include <vector>

namespace UnTech {
class ErrorList;

namespace MetaSprite {

struct FrameSetExportOrder {
    static const std::string FILE_EXTENSION;

    struct ExportName {
        idstring name;
        std::vector<NameReference> alternatives;

        bool frameExists(const NamedList<MetaSprite::Frame>& frameList) const;
        bool frameExists(const NamedList<SpriteImporter::Frame>& frameList) const;
        bool animationExists(const NamedList<Animation::Animation>& animationList) const;

        bool operator==(const ExportName& o) const
        {
            return name == o.name
                   && alternatives == o.alternatives;
        }
    };

    idstring name;
    NamedList<ExportName> stillFrames;
    NamedList<ExportName> animations;

    FrameSetExportOrder() = default;
    FrameSetExportOrder(const FrameSetExportOrder&) = delete;

    bool validate(UnTech::ErrorList& err) const;

    bool testFrameSet(const MetaSprite::FrameSet& frameSet, ErrorList& errorList) const;
    bool testFrameSet(const SpriteImporter::FrameSet& frameSet, ErrorList& errorList) const;

    bool operator==(const FrameSetExportOrder& o) const
    {
        return name == o.name
               && stillFrames == o.stillFrames
               && animations == o.animations;
    }
};

// throws exception on error
std::unique_ptr<FrameSetExportOrder> loadFrameSetExportOrder(const std::string& filename);
void saveFrameSetExportOrder(const FrameSetExportOrder& eo, const std::string& filename);
}
}
