/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resources.h"
#include "models/common/validateunique.h"
#include <cassert>

namespace UnTech {
namespace Resources {

void ResourcesFile::loadAllFiles()
{
    metaTileTilesets.loadAllFiles();
}

bool ResourcesFile::validate(ErrorList& err) const
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

    valid &= validateNamesUnique(palettes, "palettes", err);
    valid &= validateFilesAndNamesUnique(metaTileTilesets, "metatile tilesets", err);

    return valid;
}
}
}
