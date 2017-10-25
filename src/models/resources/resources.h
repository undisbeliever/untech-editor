/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "palette.h"
#include "models/common/idstring.h"
#include "models/metatiles/common.h"
#include <memory>
#include <string>
#include <vector>

namespace UnTech {
namespace Resources {

struct ResourcesFile {
    const static std::string FILE_EXTENSION;

    MetaTiles::EngineSettings metaTileEngineSettings;

    std::vector<PaletteInput> palettes;

    std::vector<std::string> metaTileTilesetFilenames;

    // raise exception if invalid
    void validate() const;

    // raise exception if palette name does not exist
    const PaletteInput& getPalette(const idstring& name) const;
};

struct ResourcesOutput {
    std::string incData;
    std::vector<uint8_t> binaryData;
};

// raises exception on error
std::unique_ptr<ResourcesFile> loadResourcesFile(const std::string& filename);

// raises exception on error
std::unique_ptr<ResourcesOutput>
compileResources(const ResourcesFile& input, const std::string& binaryFilename);
}
}
