#pragma once

#include <string>

namespace UnTech {

inline static bool isNameCharValid(const char c)
{
    return ((c == '_'
             || (c >= '0' && c <= '9')
             || (c >= 'A' && c <= 'Z')
             || (c >= 'a' && c <= 'z')));
}

inline static bool isNameValid(const std::string& name)
{
    if (name.empty()) {
        return false;
    }

    for (const char& c : name) {
        if (!isNameCharValid(c)) {
            return false;
        }
    }

    return true;
}
}
