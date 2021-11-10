/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/project/project.h"
#include <iostream>

namespace UnTech::MetaSprite::Compiler {

void writeFrameSetReferences(const Project::ProjectFile& project, StringStream& out);
void writeExportOrderReferences(const Project::ProjectFile& project, StringStream& out);

// NOTE: MUST BE CALLED LAST IN project-compiler
// NOTE: changes the ROM bank to code()
void writeActionPointFunctionTables(const NamedList<ActionPointFunction>& actionPointFunctions, StringStream& out);

}
