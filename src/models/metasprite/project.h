/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "errorlist.h"
#include "metasprite.h"
#include "spriteimporter.h"
#include "models/common/capped_vector.h"
#include <memory>

namespace UnTech {
namespace MetaSprite {

struct Project {
    static const std::string FILE_EXTENSION;

    enum class FrameSetType {
        NONE = 0,
        UNKNOWN,
        METASPRITE,
        SPRITE_IMPORTER
    };
    struct FrameSetFile {
        typedef capped_vector<FrameSetFile, MAX_FRAMESETS> list_t;

        std::string filename;

        FrameSetType type;
        std::unique_ptr<MetaSprite::FrameSet> msFrameSet;
        std::unique_ptr<SpriteImporter::FrameSet> siFrameSet;

        bool convertSpriteImporter(ErrorList& errors, bool strict = false);

        // throws an exception on error
        void loadFile();

        FrameSetFile() = default;
    };

    FrameSetFile::list_t frameSets;

    Project() = default;
    Project(const Project&) = delete;
};

std::unique_ptr<Project> loadProject(const std::string& filename);
void saveProject(const Project& project, const std::string& filename);
}
}
