/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "romdata.h"
#include "romtiledata.h"
#include "../frameset-exportorder.h"
#include "../metasprite.h"

namespace UnTech {
namespace Project {
struct ProjectFile;
}

namespace MetaSprite {
namespace Compiler {

struct CompiledRomData {
    RomTileData tileData;
    RomDmaTile16Data tilesetData;

    RomBinData paletteData;
    RomAddrTable paletteList;

    RomBinData animationData;
    RomBinData animationList;

    RomIncData frameData;
    RomBinData frameList;

    RomBinData frameObjectData;
    RomBinData tileHitboxData;
    RomBinData entityHitboxData;
    RomBinData actionPointData;

    RomIncData frameSetData;

    unsigned nFrameSets;

    bool valid = true;

    CompiledRomData(unsigned tilesetBlockSize = RomTileData::DEFAULT_TILE_BLOCK_SIZE);
    void writeToIncFile(std::ostream& out) const;
};

// Does not save tilesets or build frame data.
// Should catch all errors that the compiler will catch.
bool validateFrameSetAndBuildTilesets(const MetaSprite::FrameSet& frameSet, const FrameSetExportOrder* exportOrder,
                                      ErrorList& errorList);

void processAndSaveFrameSet(const MetaSprite::FrameSet& frameSet, const FrameSetExportOrder* exportOrder,
                            ErrorList& errorList, CompiledRomData& out);

std::unique_ptr<CompiledRomData> compileMetaSprites(const Project::ProjectFile& project, std::ostream& errorStream);
}
}
}
