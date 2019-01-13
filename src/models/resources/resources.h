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
namespace Project {
struct ProjectFile;
}
namespace Resources {

struct ResourcesOutput {
    std::string incData;
    std::vector<uint8_t> binaryData;
};

// may raise an exception
std::unique_ptr<ResourcesOutput>
compileResources(const Project::ProjectFile& input, const std::string& relativeBinaryFilename,
                 std::ostream& errorStream);
}
}
