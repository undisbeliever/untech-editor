/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "errorlist.h"
#include "externalfilelist.h"
#include "namedlist.h"
#include <algorithm>
#include <vector>

namespace UnTech {

template <class T>
inline bool validateFilesAndNamesUnique(const ExternalFileList<T>& list,
                                        const std::u8string& typeName,
                                        ErrorList& err)
{
    static const idstring countString = u8"count"_id;

    bool valid = true;

    for (auto it = list.begin(); it != list.end(); it++) {
        const std::filesystem::path& filename = it->filename;
        bool dupFn = std::any_of(it + 1, list.end(),
                                 [&](const auto& i) { return i.filename == filename; });
        if (dupFn) {
            err.addErrorString(u8"Duplicate ", typeName, u8" file detected: ", filename.string());
            valid = false;
            continue;
        }

        if (it->value == nullptr) {
            continue;
        }

        const idstring& name = it->value->name;
        if (name.isValid() == false) {
            auto d = std::distance(list.begin(), it);
            err.addErrorString(u8"Missing name in ", typeName, u8" ", d);
            valid = false;
            continue;
        }

        if (name == countString) {
            err.addErrorString(u8"Invalid ", typeName, u8" name: count");
            valid = false;
            continue;
        }

        bool dup = std::any_of(it + 1, list.end(),
                               [&](const auto& i) { return i.value && i.value->name == name; });
        if (dup) {
            err.addErrorString(u8"Duplicate ", typeName, u8" name detected: ", name);
            valid = false;
        }
    }

    return valid;
}

template <class T, typename ErrorFunction>
inline bool validateNamesUnique(const NamedList<T>& list,
                                const std::u8string& typeName,
                                ErrorFunction err)
{
    static const idstring countString = u8"count"_id;

    bool valid = true;
    auto addError = [&](const unsigned index, const auto... message) {
        err(index, message...);
        valid = false;
    };

    for (auto [index, item] : const_enumerate(list)) {
        const idstring& name = item.name;
        if (name.isValid() == false) {
            addError(index, u8"Missing name in ", typeName, u8" ", index);
            continue;
        }

        if (name == countString) {
            addError(index, u8"Invalid ", typeName, u8" name: count");
            continue;
        }

        bool dup = std::any_of(list.begin(), list.begin() + index,
                               [&](const auto& i) { return i.name == name; });
        if (dup) {
            addError(index, u8"Duplicate ", typeName, u8" name detected: ", name);
        }
    }

    return valid;
}

}
