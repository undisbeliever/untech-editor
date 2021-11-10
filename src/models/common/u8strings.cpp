/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "u8strings.h"
#include <iostream>

namespace UnTech {

std::u8string convert_old_string(const char* str)
{
    const std::string_view sv(str);
    return std::u8string(sv.begin(), sv.end());
}

void stdout_write(std::u8string_view s)
{
    std::cout.write(reinterpret_cast<const char*>(s.data()), s.size());
}

void stderr_write(std::u8string_view s)
{
    std::cerr.write(reinterpret_cast<const char*>(s.data()), s.size());
}

}
