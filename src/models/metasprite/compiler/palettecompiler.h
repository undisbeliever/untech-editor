/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "romdata.h"
#include "../metasprite.h"
#include <array>
#include <vector>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

struct CompiledRomData;

using CompiledPalette = std::array<uint8_t, 30>;

std::vector<CompiledPalette> processPalettes(const std::vector<Snes::Palette4bpp>& palettes);

uint16_t savePalettes(const std::vector<CompiledPalette>& palettes, CompiledRomData& out);

}
}
}
