/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "metasprite.h"
#include "spriteimporter.h"
#include <memory>

namespace UnTech {
namespace MetaSprite {

struct FrameSetFile {
    enum class FrameSetType {
        NONE = 0,
        UNKNOWN,
        METASPRITE,
        SPRITE_IMPORTER
    };

    std::string filename;

    FrameSetType type = FrameSetType::NONE;
    std::unique_ptr<MetaSprite::FrameSet> msFrameSet;
    std::unique_ptr<SpriteImporter::FrameSet> siFrameSet;

    void setTypeFromExtension();

    bool convertSpriteImporter(ErrorList& errors, bool strict = false);

    // throws an exception on error
    void loadFile();

    // returns and empty idstring if no frameSet exists
    const idstring& name() const;

    FrameSetFile() = default;
};

bool validateFrameSetNamesUnique(const std::vector<FrameSetFile>& frameSets,
                                 ErrorList& err);

}
}
