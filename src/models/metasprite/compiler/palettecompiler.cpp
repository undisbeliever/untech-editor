/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palettecompiler.h"
#include "compiler.h"
#include "models/common/iterators.h"

namespace UnTech::MetaSprite::Compiler {

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
        for (const auto i : range(1, palette.size())) {
            uint16_t cData = palette.at(i).data();
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
    std::vector<uint8_t> indexes(palettes.size() * 2);

    auto it = indexes.begin();

    for (const auto& pData : palettes) {
        uint16_t offset = out.paletteData.addData_Index(pData);

        *it++ = offset & 0xff;
        *it++ = (offset >> 8) & 0xff;
    }
    assert(it == indexes.end());

    return out.paletteList.addData_Index(indexes);
}

}
