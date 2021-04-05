/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "stringbuilder.h"
#include <charconv>

namespace UnTech::StringBuilder {

template <typename T>
static void concat_int(std::string& str, const T value)
{
    const size_t initialSize = str.size();
    constexpr size_t bSize = stringSize(T());

    str.resize(initialSize + bSize);

    char* start = str.data();
    char* ptr = start + initialSize;
    char* end = ptr + bSize;

    auto r = std::to_chars(ptr, end, value);
    assert(r.ec == std::errc());

    str.resize(r.ptr - start);
}

void concat(std::string& str, int64_t value)
{
    concat_int(str, value);
}

void concat(std::string& str, uint64_t value)
{
    concat_int(str, value);
}

void concat(std::string& str, int32_t value)
{
    concat_int(str, value);
}

void concat(std::string& str, uint32_t value)
{
    concat_int(str, value);
}

}
