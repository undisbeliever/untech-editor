#pragma once

#include "romdata.h"
#include "../metasprite.h"

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

class PaletteCompiler {
public:
    PaletteCompiler();
    PaletteCompiler(const PaletteCompiler&) = delete;

    void writeToIncFile(std::ostream& out) const;

    RomOffsetPtr process(const MetaSprite::FrameSet& frameSet);

private:
    RomBinData _paletteData;
    RomAddrTable _paletteList;
};
}
}
}
