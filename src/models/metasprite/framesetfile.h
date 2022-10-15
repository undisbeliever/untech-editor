/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "metasprite.h"
#include "spriteimporter.h"
#include <filesystem>
#include <memory>

namespace UnTech::MetaSprite {

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

    // throws an exception on error
    void loadFile();

    // returns an empty idstring if no frameSet exists
    [[nodiscard]] const idstring& name() const;
    [[nodiscard]] const idstring& exportOrder() const;

    FrameSetFile() = default;

    // Cannot use default here.  I want std::unique_ptr value comparison.
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

}
