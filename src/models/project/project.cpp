/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project.h"
#include "models/common/errorlist.h"
#include "models/common/validateunique.h"
#include <cassert>

namespace UnTech {
namespace Project {

void ProjectFile::loadAllFiles()
{
    metaTileTilesets.loadAllFiles();
    frameSetExportOrders.loadAllFiles();
    rooms.loadAllFiles();

    for (auto& fs : frameSets) {
        fs.loadFile();
    }
}

bool MemoryMapSettings::validate(ErrorList& err) const
{
    bool valid = true;

    auto validateMinMax = [&](unsigned value, unsigned min, unsigned max, const char* msg) {
        if (value < min || value > max) {
            err.addErrorString(msg, " (", value, ", min: ", min, ", max: ", max, ")");
            valid = false;
        }
    };

    validateMinMax(firstBank, 0, 255, "Invalid firstBank");

    if (mode == MappingMode::HIROM) {
        const unsigned maximumNBanks = std::min<unsigned>(64, 256 - firstBank);
        validateMinMax(nBanks, 1, maximumNBanks, "Invalid nBanks");
    }
    else if (mode == MappingMode::LOROM) {
        const unsigned maximumNBanks = std::min<unsigned>(128, 256 - firstBank);
        validateMinMax(nBanks, 1, maximumNBanks, "Invalid nBanks");
    }
    else {
        err.addErrorString("Invalid mapping mode");
    }

    const unsigned lastBank = firstBank + nBanks;
    auto inBounds = [&](unsigned a) { return a >= firstBank && a <= lastBank; };

    if (inBounds(0x7e) || inBounds(0x7f)) {
        err.addErrorString("Invalid memory map: Work RAM inside mapping");
    }

    return valid;
}

bool ProjectFile::validate(ErrorList& err) const
{
    bool valid = true;

    valid &= memoryMap.validate(err);
    valid &= roomSettings.validate(err);

    if (frameSetExportOrders.size() > MetaSprite::MAX_EXPORT_NAMES) {
        err.addErrorString("Too many MetaSprite export orders");
    }
    if (frameSets.size() > MetaSprite::MAX_FRAMESETS) {
        err.addErrorString("Too many MetaSprite FrameSets");
    }

    valid &= validateNamesUnique(palettes, "palettes", err);
    valid &= validateFilesAndNamesUnique(metaTileTilesets, "metatile tilesets", err);

    valid &= MetaSprite::validateFrameSetNamesUnique(frameSets, err);
    valid &= validateFilesAndNamesUnique(frameSetExportOrders, "export order", err);

    valid &= validateFilesAndNamesUnique(rooms, "Room", err);

    return valid;
}

}
}
