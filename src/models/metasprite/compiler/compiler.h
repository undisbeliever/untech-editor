/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "romdata.h"
#include "romtiledata.h"
#include "../errorlist.h"
#include "../metasprite.h"
#include "../project.h"

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

struct CompiledRomData {
    RomTileData tileData;
    RomIncData tilesetData;

    RomBinData paletteData;
    RomAddrTable paletteList;

    RomBinData animationData;
    RomAddrTable animationList;

    RomIncData frameData;
    RomAddrTable frameList;

    RomBinData frameObjectData;
    RomBinData tileHitboxData;
    RomBinData entityHitboxData;
    RomBinData actionPointData;

    RomIncData frameSetData;
    RomAddrTable frameSetList;

    CompiledRomData(unsigned tilesetBlockSize = RomTileData::DEFAULT_TILE_BLOCK_SIZE);
    void writeToIncFile(std::ostream& out) const;
};

void processAndSaveFrameSet(const MetaSprite::FrameSet& frameSet, const FrameSetExportOrder* exportOrder,
                            ErrorList& errorList, CompiledRomData& out);

void processProject(Project& project, ErrorList& errorList, CompiledRomData& out);

}
}
}
