/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resources.h"
#include "models/common/validatorhelper.h"
#include <algorithm>
#include <cassert>
#include <functional>
#include <sstream>
#include <unordered_set>

namespace UnTech {
namespace Resources {

template <class T>
static void validateNamesUnique(const std::vector<T>& inputList, const std::string& typeName)
{
    std::unordered_set<idstring> nameSet;

    for (const T& item : inputList) {
        const std::string& name = item.name;
        if (name == "count") {
            throw std::runtime_error("Invalid " + typeName + " name: count");
        }

        auto it = nameSet.find(name);
        if (it != nameSet.end()) {
            throw std::runtime_error("Duplicate " + typeName + " name detected: " + name);
        }
        nameSet.insert(name);
    }
}

void ResourcesFile::validate() const
{
    validateMinMax(blockSize, 1024, 64 * 1024, "block size invalid");
    validateMinMax(blockCount, 1, 128, "block count invalid");

    validateNamesUnique(palettes, "palettes");
}

const PaletteInput& ResourcesFile::getPalette(const idstring& name) const
{

    auto it = std::find_if(palettes.begin(), palettes.end(),
                           [&](const auto& p) { return p.name == name; });

    if (it == palettes.end()) {
        throw std::runtime_error("Cannot find palette: " + name);
    }

    return *it;
}
}
}
