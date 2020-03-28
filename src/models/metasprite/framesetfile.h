/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "metasprite.h"
#include "spriteimporter.h"
#include <filesystem>
#include <memory>

namespace UnTech {
namespace MetaSprite {

struct FrameSetFile {
    enum class FrameSetType {
        UNKNOWN,
        METASPRITE,
        SPRITE_IMPORTER
    };

    std::filesystem::path filename;

    FrameSetType type = FrameSetType::UNKNOWN;
    std::unique_ptr<MetaSprite::FrameSet> msFrameSet;
    std::unique_ptr<SpriteImporter::FrameSet> siFrameSet;

    void setTypeFromExtension();

    bool convertSpriteImporter(ErrorList& errors, bool strict = false);

    // throws an exception on error
    void loadFile();

    // returns an empty idstring if no frameSet exists
    const idstring& name() const;
    const idstring& exportOrder() const;

    FrameSetFile() = default;

    bool operator==(const FrameSetFile& o) const
    {
        if (filename != o.filename
            || type != o.type) {

            return false;
        }
        if (siFrameSet && o.siFrameSet) {
            return *siFrameSet == *o.siFrameSet;
        }
        if (msFrameSet && o.msFrameSet) {
            return *msFrameSet == *o.msFrameSet;
        }
        return siFrameSet == nullptr && msFrameSet == nullptr
               && o.siFrameSet == nullptr && o.msFrameSet == nullptr;
    }
};

bool validateFrameSetNamesUnique(const std::vector<FrameSetFile>& frameSets,
                                 ErrorList& err);

}
}
