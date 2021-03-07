/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "framesetexportlist.h"
#include "romdata.h"
#include <vector>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

struct CompiledRomData;

std::vector<std::vector<uint8_t>> processAnimations(const FrameSetExportList& exportList);

uint16_t saveAnimations(const std::vector<std::vector<uint8_t>>& animations, CompiledRomData& out);

}
}
}
