/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2023, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "compiler-status.h"
#include "project-data.h"

namespace UnTech::Project {

bool compileResources_impl(CompilerStatus& status, ProjectData& data, const ProjectFile& project, const bool earlyExit);

inline bool compileResources(CompilerStatus& status, ProjectData& data, const ProjectFile& pf)
{
    return compileResources_impl(status, data, pf, false);
}

inline bool compileResources_earlyExit(CompilerStatus& status, ProjectData& data, const ProjectFile& pf)
{
    return compileResources_impl(status, data, pf, true);
}

}
