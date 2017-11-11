/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "error-list.h"
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

    unsigned blockSize;
    unsigned blockCount;

    MetaTiles::EngineSettings metaTileEngineSettings;

    std::vector<std::shared_ptr<PaletteInput>> palettes;

    std::vector<std::string> metaTileTilesetFilenames;

    bool validate(ErrorList& err) const;

    // returns nullptr if name does not exist
    std::shared_ptr<const PaletteInput> getPalette(const idstring& name) const;

    bool operator==(const ResourcesFile& o) const
    {
        return blockSize == o.blockSize
               && blockCount == o.blockCount
               && metaTileEngineSettings == o.metaTileEngineSettings
               && palettes == o.palettes
               && metaTileTilesetFilenames == o.metaTileTilesetFilenames;
    }
    bool operator!=(const ResourcesFile& o) const { return !(*this == o); }
};

struct ResourcesOutput {
    std::string incData;
    std::vector<uint8_t> binaryData;
};

// raises exception on error
std::unique_ptr<ResourcesFile> loadResourcesFile(const std::string& filename);

// may raise an exception
std::unique_ptr<ResourcesOutput>
compileResources(const ResourcesFile& input, const std::string& relativeBinaryFilename,
                 std::ostream& errorStream);
}
}
