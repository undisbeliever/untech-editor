/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resources.h"
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
    validateNamesUnique(palettes, "palettes");
}
}
}
