/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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

    for (auto& fs : frameSets) {
        fs.loadFile();
    }
}

bool ProjectFile::validate(ErrorList& err) const
{
    bool valid = true;

    auto validateMinMax = [&](unsigned value, unsigned min, unsigned max, const char* msg) {
        if (value < min || value > max) {
            err.addError(msg
                         + std::string(" (") + std::to_string(value)
                         + ", min: " + std::to_string(min)
                         + ", max: " + std::to_string(max) + ")");
            valid = false;
        }
    };

    validateMinMax(blockSettings.size, 1024, 64 * 1024, "block size invalid");
    validateMinMax(blockSettings.count, 1, 128, "block count invalid");

    if (frameSetExportOrders.size() > MetaSprite::MAX_EXPORT_NAMES) {
        err.addError("Too many MetaSprite export orders");
    }
    if (frameSets.size() > MetaSprite::MAX_FRAMESETS) {
        err.addError("Too many MetaSprite FrameSets");
    }

    valid &= validateNamesUnique(palettes, "palettes", err);
    valid &= validateFilesAndNamesUnique(metaTileTilesets, "metatile tilesets", err);

    valid &= MetaSprite::validateFrameSetNamesUnique(frameSets, err);
    valid &= validateFilesAndNamesUnique(frameSetExportOrders, "export order", err);

    return valid;
}
}
}
