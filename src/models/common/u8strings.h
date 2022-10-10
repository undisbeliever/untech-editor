/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <string>

namespace UnTech {

// ASSUMES string is utf8
std::u8string convert_old_string(const char* str);
std::u8string convert_old_string(std::string_view sv);

void stdout_write(std::u8string_view s);
void stderr_write(std::u8string_view s);

}
