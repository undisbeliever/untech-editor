/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "romdata.h"
#include "romtiledata.h"
#include "../frameset-exportorder.h"
#include "../metasprite.h"

namespace UnTech::Project {
struct ProjectFile;
}

namespace UnTech::MetaSprite::Compiler {

struct FrameData;
struct FrameSetData;

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
    RomBinData actionPointData;
    RomBinData collisionBoxData;

    RomBinData frameSetData;

    bool valid = true;

    explicit CompiledRomData(const Project::MemoryMapSettings& memoryMap);

    void addFrameSetData(const FrameSetData& fsData);
};

}
