#ifndef _UNTECH_MODELS_COMMON_NAMEDCHECKS_H_
#define _UNTECH_MODELS_COMMON_NAMEDCHECKS_H_

#include <string>

namespace UnTech {

inline static bool isNameListCharValid(const char c)
{
    return ((c == '_'
             || (c >= '0' && c <= '9')
             || (c >= 'A' && c <= 'Z')
             || (c >= 'a' && c <= 'z')));
}

inline static bool isNameListNameValid(const std::string& name)
{
    if (name.empty()) {
        return false;
    }

    for (const char& c : name) {
        if (!isNameListCharValid(c)) {
            return false;
        }
    }

    return true;
}
}

#endif
