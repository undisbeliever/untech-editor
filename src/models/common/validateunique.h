/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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
                                        const std::string& typeName,
                                        ErrorList& err)
{
    const idstring countString("count");

    bool valid = true;

    for (auto it = list.begin(); it != list.end(); it++) {
        const std::string& filename = it->filename;
        bool dupFn = std::any_of(it + 1, list.end(),
                                 [&](const auto& i) { return i.filename == filename; });
        if (dupFn) {
            err.addError("Duplicate " + typeName + " file detected: " + filename);
            valid = false;
            continue;
        }

        if (it->value == nullptr) {
            continue;
        }

        const idstring& name = it->value->name;
        if (name.isValid() == false) {
            auto d = std::distance(list.begin(), it);
            err.addError("Missing name in " + typeName + ' ' + std::to_string(d));
            valid = false;
            continue;
        }

        if (name == countString) {
            err.addError("Invalid " + typeName + " name: count");
            valid = false;
            continue;
        }

        bool dup = std::any_of(it + 1, list.end(),
                               [&](const auto& i) { return i.value && i.value->name == name; });
        if (dup) {
            err.addError("Duplicate " + typeName + " name detected: " + name);
            valid = false;
        }
    }

    return valid;
}

template <class T>
inline bool validateNamesUnique(const NamedList<T>& list,
                                const std::string& typeName,
                                ErrorList& err)
{
    const idstring countString("count");

    bool valid = true;

    for (auto it = list.begin(); it != list.end(); it++) {
        if (*it == nullptr) {
            continue;
        }

        const idstring& name = (*it)->name;
        if (name.isValid() == false) {
            auto d = std::distance(list.begin(), it);
            err.addError("Missing name in " + typeName + ' ' + std::to_string(d));
            valid = false;
            continue;
        }

        if (name == countString) {
            err.addError("Invalid " + typeName + " name: count");
            valid = false;
            continue;
        }

        bool dup = std::any_of(it + 1, list.end(),
                               [&](const auto& i) { return i && i->name == name; });
        if (dup) {
            err.addError("Duplicate " + typeName + " name detected: " + name);
            valid = false;
        }
    }

    return valid;
}

template <class T>
inline bool validateNamesUnique(const std::vector<T>& list,
                                const std::string& typeName,
                                ErrorList& err)
{
    const idstring countString("count");

    bool valid = true;

    for (auto it = list.begin(); it != list.end(); it++) {
        const idstring& name = it->name;
        if (name.isValid() == false) {
            auto d = std::distance(list.begin(), it);
            err.addError("Missing name in " + typeName + ' ' + std::to_string(d));
            valid = false;
            continue;
        }

        if (name == countString) {
            err.addError("Invalid " + typeName + " name: count");
            valid = false;
            continue;
        }

        bool dup = std::any_of(it + 1, list.end(),
                               [&](const auto& i) { return i.name == name; });
        if (dup) {
            err.addError("Duplicate " + typeName + " name detected: " + name);
            valid = false;
        }
    }

    return valid;
}
}
