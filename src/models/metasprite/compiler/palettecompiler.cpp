/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palettecompiler.h"
#include "compiler.h"

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

std::vector<CompiledPalette> processPalettes(const std::vector<Snes::Palette4bpp>& palettes)
{
    assert(palettes.size() <= MAX_PALETTES);
    assert(palettes.empty() == false);

    std::vector<CompiledPalette> ret(palettes.size());
    auto retIt = ret.begin();

    for (const auto& palette : palettes) {
        CompiledPalette& pData = *retIt++;

        // Color 0 is always transparent and thus not saved to ROM
        auto pIt = pData.begin();
        for (unsigned i = 1; i < palette.N_COLORS; i++) {
            uint16_t cData = palette.color(i).data();
            *pIt++ = cData & 0xff;
            *pIt++ = cData >> 8;
        }
        assert(pIt == pData.end());
    }
    assert(retIt == ret.end());

    return ret;
}

uint16_t savePalettes(const std::vector<CompiledPalette>& palettes, CompiledRomData& out)
{
    std::vector<uint32_t> offsets;
    offsets.reserve(palettes.size());

    for (const auto& pData : palettes) {
        offsets.emplace_back(out.paletteData.addData_Index(pData));
    }

    return out.paletteList.getOrInsertTable(offsets);
}

}
}
}
