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

bool BlockSettings::validate(ErrorList& err) const
{
    bool valid = true;

    auto validateMinMax = [&](unsigned value, unsigned min, unsigned max, const char* msg) {
        if (value < min || value > max) {
            err.addErrorString(msg, " (", value, ", min: ", min, ", max: ", max, ")");
            valid = false;
        }
    };

    validateMinMax(size, 1024, 64 * 1024, "block size invalid");
    validateMinMax(count, 1, 128, "block count invalid");

    return valid;
}

bool ProjectFile::validate(ErrorList& err) const
{
    bool valid = true;

    valid &= blockSettings.validate(err);

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
