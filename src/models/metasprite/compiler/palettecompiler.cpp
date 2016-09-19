#include "palettecompiler.h"
#include "models/snes/palette.hpp"

using namespace UnTech::MetaSprite;
using namespace UnTech::MetaSprite::Compiler;
namespace MS = UnTech::MetaSprite::MetaSprite;

PaletteCompiler::PaletteCompiler()
    : _paletteData("PD", "MS_PaletteData")
    , _paletteList("PL", "MS_PaletteList", "PD")
{
}

void PaletteCompiler::writeToIncFile(std::ostream& out) const
{
    _paletteData.writeToIncFile(out);
    _paletteList.writeToIncFile(out);
}

RomOffsetPtr PaletteCompiler::process(const MS::FrameSet& frameSet)
{
    // Some framesets can have the same palette, will check for duplicates
    const auto& palettes = frameSet.palettes;

    assert(palettes.size() <= MAX_PALETTES);

    if (palettes.size() == 0) {
        throw std::runtime_error("No Palettes in Frameset");
    }

    std::vector<uint32_t> offsets;
    offsets.reserve(palettes.size());

    for (const auto& palette : palettes) {
        std::vector<uint8_t> paletteData = palette.paletteData();

        // Remove transparent color, saves 2 bytes in ROM.
        paletteData.erase(paletteData.begin(), paletteData.begin() + 2);

        offsets.emplace_back(_paletteData.addData(paletteData).offset);
    }

    return _paletteList.getOrInsertTable(offsets);
}
