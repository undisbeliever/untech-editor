/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project.h"
#include "models/common/errorlist.h"
#include "models/common/validateunique.h"
#include <cassert>

namespace UnTech::Project {

void ProjectFile::loadAllFiles()
{
    metaTileTilesets.loadAllFiles();
    frameSetExportOrders.loadAllFiles();
    rooms.loadAllFiles();

    for (auto& fs : frameSets) {
        fs.loadFile();
    }
}

void ProjectFile::loadAllFilesIgnoringErrors()
{
    auto loadFiles = [](auto& list) {
        for (auto& item : list) {
            try {
                item.loadFile();
            }
            catch (std::exception& ex) {
                // ignore error
            }
        }
    };

    loadFiles(metaTileTilesets);
    loadFiles(frameSetExportOrders);
    loadFiles(rooms);
    loadFiles(frameSets);
}

static bool validate(const MemoryMapSettings& input, ErrorList& err)
{
    bool valid = true;

    auto validateMinMax = [&](unsigned value, unsigned min, unsigned max, const char8_t* msg) {
        if (value < min || value > max) {
            err.addErrorString(msg, u8" (", value, u8", min: ", min, u8", max: ", max, u8")");
            valid = false;
        }
    };

    validateMinMax(input.firstBank, 0, 255, u8"Invalid firstBank");

    if (input.mode == MappingMode::HIROM) {
        const unsigned maximumNBanks = std::min<unsigned>(64, 256 - input.firstBank);
        validateMinMax(input.nBanks, 1, maximumNBanks, u8"Invalid nBanks");
    }
    else if (input.mode == MappingMode::LOROM) {
        const unsigned maximumNBanks = std::min<unsigned>(128, 256 - input.firstBank);
        validateMinMax(input.nBanks, 1, maximumNBanks, u8"Invalid nBanks");
    }
    else {
        err.addErrorString(u8"Invalid mapping mode");
    }

    const unsigned lastBank = input.firstBank + input.nBanks;
    auto inBounds = [&](unsigned a) { return a >= input.firstBank && a <= lastBank; };

    if (inBounds(0x7e) || inBounds(0x7f)) {
        err.addErrorString(u8"Invalid memory map: Work RAM inside mapping");
    }

    return valid;
}

bool validateProjectSettings(const ProjectSettings& input, ErrorList& err)
{
    bool valid = true;

    valid &= validate(input.memoryMap, err);
    valid &= validate(input.roomSettings, err);

    return valid;
}

}
