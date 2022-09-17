/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <filesystem>
#include <memory>
#include <vector>

namespace UnTech {
class StringStream;
}

namespace UnTech::Project {

struct ProjectFile;

struct ProjectOutput {
    std::u8string incData;
    std::vector<uint8_t> binaryData;
};

// may raise an exception
std::unique_ptr<ProjectOutput>
compileProject(const ProjectFile& input, const std::filesystem::path& relativeBinFilename,
               StringStream& errorStream);
}
