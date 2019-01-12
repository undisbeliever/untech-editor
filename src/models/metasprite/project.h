/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "frameset-exportorder.h"
#include "framesetfile.h"
#include "models/common/externalfilelist.h"
#include <memory>

namespace UnTech {
namespace MetaSprite {

struct Project {
    static const std::string FILE_EXTENSION;

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
