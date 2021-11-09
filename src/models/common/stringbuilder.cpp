/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "stringbuilder.h"
#include <charconv>

namespace UnTech::StringBuilder {

template <typename T>
static char* concat_int(char* ptr, char* const end, const T value)
{
    auto r = std::to_chars(ptr, end, value);
    assert(r.ec == std::errc());

    return r.ptr;
}

char* concat(char* ptr, char* const end, int32_t value)
{
    return concat_int(ptr, end, value);
}

char* concat(char* ptr, char* const end, int64_t value)
{
    return concat_int(ptr, end, value);
}

char* concat(char* ptr, char* const end, uint32_t value)
{
    return concat_int(ptr, end, value);
}

char* concat(char* ptr, char* const end, uint64_t value)
{
    return concat_int(ptr, end, value);
}

}
