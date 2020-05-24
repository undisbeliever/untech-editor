/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
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
    static const int METASPRITE_FORMAT_VERSION;

    RomTileData tileData;
    RomBinData dmaTile16Data;

    RomBinData paletteData;
    RomBinData paletteList;

    RomBinData animationData;
    RomBinData animationList;

    RomBinData frameData;
    RomBinData frameList;

    RomBinData frameObjectData;
    RomBinData entityHitboxData;
    RomBinData actionPointData;

    RomBinData frameSetData;

    bool valid = true;

    CompiledRomData(const Project::MemoryMapSettings& memoryMap);
};

// Does not save tilesets or build frame data.
// Should catch all errors that the compiler will catch.
// exportOrder can be null
bool validateFrameSetAndBuildTilesets(const MetaSprite::FrameSet& frameSet, const FrameSetExportOrder* exportOrder,
                                      const ActionPointMapping& actionPointMapping,
                                      ErrorList& errorList);

// exportOrder can be null
void processAndSaveFrameSet(const MetaSprite::FrameSet& frameSet, const FrameSetExportOrder* exportOrder,
                            const ActionPointMapping& actionPointMapping,
                            ErrorList& errorList, CompiledRomData& out);

std::unique_ptr<CompiledRomData> compileMetaSprites(const Project::ProjectFile& project, std::ostream& errorStream);
}
}
}
