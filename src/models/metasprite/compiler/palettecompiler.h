/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

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
