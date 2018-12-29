/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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

    if (palettes.size() == 0) {
        throw std::runtime_error("No Palettes in Frameset");
    }

    std::vector<CompiledPalette> ret;
    ret.reserve(palettes.size());

    for (const auto& palette : palettes) {
        ret.emplace_back(30);
        std::vector<uint8_t>& pData = ret.back();

        // Color 0 is always transparent and thus not saved to ROM
        auto pIt = pData.begin();
        for (unsigned i = 1; i < palette.N_COLORS; i++) {
            uint16_t cData = palette.color(i).data();
            *pIt++ = cData & 0xff;
            *pIt++ = cData >> 8;
        }
        assert(pIt == pData.end());
    }

    return ret;
}

RomOffsetPtr savePalettes(const std::vector<CompiledPalette>& palettes, CompiledRomData& out)
{
    std::vector<uint32_t> offsets;
    offsets.reserve(palettes.size());

    for (const auto& pData : palettes) {
        offsets.emplace_back(out.paletteData.addData(pData).offset);
    }

    return out.paletteList.getOrInsertTable(offsets);
}

}
}
}
