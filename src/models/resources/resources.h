/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "palette.h"
#include "models/common/externalfilelist.h"
#include "models/common/idstring.h"
#include "models/common/namedlist.h"
#include "models/metatiles/common.h"
#include "models/metatiles/metatile-tileset.h"
#include <memory>
#include <string>
#include <vector>

namespace UnTech {
namespace Resources {

struct BlockSettings {
    unsigned size;
    unsigned count;

    bool operator==(const BlockSettings& o) const
    {
        return size == o.size
               && count == o.count;
    }
    bool operator!=(const BlockSettings& o) const { return !(*this == o); }
};

struct ResourcesFile {
    const static std::string FILE_EXTENSION;

    BlockSettings blockSettings;
    MetaTiles::EngineSettings metaTileEngineSettings;

    NamedList<PaletteInput> palettes;
    ExternalFileList<MetaTiles::MetaTileTilesetInput> metaTileTilesets;

    void loadAllFiles();

    bool validate(ErrorList& err) const;

    bool operator==(const ResourcesFile& o) const
    {
        return blockSettings == o.blockSettings
               && metaTileEngineSettings == o.metaTileEngineSettings
               && palettes == o.palettes
               && metaTileTilesets == o.metaTileTilesets;
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
