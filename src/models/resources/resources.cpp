/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resources.h"
#include <algorithm>
#include <cassert>
#include <functional>
#include <sstream>
#include <unordered_set>

namespace UnTech {
namespace Resources {

void ResourcesFile::loadAllFiles()
{
    metaTileTilesets.loadAllFiles();
}

template <class T>
static bool validateNamesUnique(const ExternalFileList<T>& inputList,
                                const std::string& typeName,
                                ErrorList& err)
{
    bool valid = true;

    std::unordered_set<idstring> nameSet;

    for (const ExternalFileItem<T>& item : inputList) {
        if (item.value == nullptr) {
            continue;
        }

        const std::string& name = item.value->name;
        if (name == "count") {
            err.addError("Invalid " + typeName + " name: count");
            valid = false;
        }

        auto it = nameSet.find(name);
        if (it != nameSet.end()) {
            err.addError("Duplicate " + typeName + " name detected: " + name);
            valid = false;
        }
        nameSet.insert(name);
    }

    return valid;
}

template <class T>
static bool validateNamesUnique(const NamedList<T>& inputList,
                                const std::string& typeName,
                                ErrorList& err)
{
    bool valid = true;

    std::unordered_set<idstring> nameSet;

    for (const std::unique_ptr<T>& item : inputList) {
        const std::string& name = item->name;
        if (name == "count") {
            err.addError("Invalid " + typeName + " name: count");
            valid = false;
        }

        auto it = nameSet.find(name);
        if (it != nameSet.end()) {
            err.addError("Duplicate " + typeName + " name detected: " + name);
            valid = false;
        }
        nameSet.insert(name);
    }

    return valid;
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
    valid &= validateNamesUnique(metaTileTilesets, "metatile tilesets", err);

    return valid;
}
}
}
