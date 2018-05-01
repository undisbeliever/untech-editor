/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "errorlist.h"
#include "frameset-exportorder.h"
#include "metasprite.h"
#include "spriteimporter.h"
#include "models/common/externalfilelist.h"
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
        std::string filename;

        FrameSetType type;
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

    std::vector<FrameSetFile> frameSets;
    ExternalFileList<FrameSetExportOrder> exportOrders;

    Project() = default;
    Project(const Project&) = delete;

    bool validateNamesUnique(ErrorList& errors) const;
};

std::unique_ptr<Project> loadProject(const std::string& filename);
void saveProject(const Project& project, const std::string& filename);
}
}
