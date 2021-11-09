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

//  UnTech::StringBuilder::hex<0> has no padding
char* concat(char* ptr, char* const end, const hex<0> value)
{
    static_assert(std::is_same_v<decltype(value.value), const uint32_t>);

    auto r = std::to_chars(ptr, end, value.value, 16);
    assert(r.ec == std::errc());

    return r.ptr;
}

// UnTech::StringBuilder::hex<N> has padding
template <unsigned N>
char* concat(char* buffer, char* const end, const hex<N> value)
{
    static_assert(std::is_same_v<decltype(value.value), const uint32_t>);

    constexpr size_t bSize = N;
    static_assert(bSize == stringSize(hex<N>(uint32_t(0))));

    char* ptr = buffer + bSize;

    uint32_t v = value.value;

    for (size_t i = 0; i < bSize; i++) {
        const auto digit = v % 16;
        v >>= 4;

        *--ptr = digit < 10 ? digit + '0'
                            : digit + 'a' - 10;
    }
    assert(ptr <= end);

    return buffer + bSize;
}
template char* concat(char*, char* const, const hex<4>);
template char* concat(char*, char* const, const hex<6>);
template char* concat(char*, char* const, const hex<8>);

}
