/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <algorithm>
#include <string_view>

namespace UnTech::Xml {

inline bool isName(const char8_t c)
{
    return ((c >= u8'A' && c <= u8'Z')
            || (c >= u8'a' && c <= u8'z')
            || (c >= u8'0' && c <= u8'9')
            || c == u8'.'
            || c == u8':'
            || c == u8'-'
            || c == u8'_');
}

inline bool isName(const std::u8string_view text)
{
    return std::all_of(text.begin(), text.end(),
                       [](const char8_t c) { return isName(c); });
}

}
