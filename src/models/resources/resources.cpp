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

template <class T>
static bool validateNamesUnique(const std::vector<std::shared_ptr<T>>& inputList,
                                const std::string& typeName,
                                ErrorList& err)
{
    bool valid = true;

    std::unordered_set<idstring> nameSet;

    for (const std::shared_ptr<T>& item : inputList) {
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

    validateMinMax(blockSize, 1024, 64 * 1024, "block size invalid");
    validateMinMax(blockCount, 1, 128, "block count invalid");

    valid &= validateNamesUnique(palettes, "palettes", err);

    return valid;
}

std::shared_ptr<const PaletteInput> ResourcesFile::getPalette(const idstring& name) const
{
    auto it = std::find_if(palettes.begin(), palettes.end(),
                           [&](const auto& p) { return p->name == name; });

    if (it == palettes.end()) {
        return nullptr;
    }
    return *it;
}
}
}
