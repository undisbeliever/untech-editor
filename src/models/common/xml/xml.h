/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <string>

namespace UnTech {
namespace Xml {

std::string escape(const std::string_view text, bool intag = true);

inline bool isName(char c)
{
    return ((c >= 'A' && c <= 'Z')
            || (c >= 'a' && c <= 'z')
            || (c >= '0' && c <= '9')
            || c == '.'
            || c == ':'
            || c == '-'
            || c == '_');
}

inline bool isName(const std::string_view text)
{
    for (const char c : text) {
        if (isName(c) == false) {
            return false;
        }
    }
    return true;
}
}
}
