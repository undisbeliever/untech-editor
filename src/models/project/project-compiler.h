/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>
#include <sstream>
#include <vector>

namespace UnTech {
namespace Project {
struct ProjectFile;

struct ProjectOutput {
    std::stringstream incData;
    std::vector<uint8_t> binaryData;
};

// may raise an exception
std::unique_ptr<ProjectOutput>
compileProject(const ProjectFile& input, const std::string& relativeBinaryFilename,
               std::ostream& errorStream);
}
}
