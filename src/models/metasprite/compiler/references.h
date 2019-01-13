/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/project/project.h"
#include <iostream>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

void writeFrameSetReferences(const Project::ProjectFile& project, std::ostream& out);
void writeExportOrderReferences(const Project::ProjectFile& project, std::ostream& out);

}
}
}
