/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "stringbuilder.h"
#include <charconv>

namespace UnTech::StringBuilder {

template <typename T>
static char8_t* concat_int(char8_t* ptr, char8_t* const end, const T value)
{
    static_assert(sizeof(char8_t) == sizeof(char));

    auto r = std::to_chars(reinterpret_cast<char*>(ptr), reinterpret_cast<char*>(end), value);
    assert(r.ec == std::errc());

    return reinterpret_cast<char8_t*>(r.ptr);
}

char8_t* concat(char8_t* ptr, char8_t* const end, int32_t value)
{
    return concat_int(ptr, end, value);
}

char8_t* concat(char8_t* ptr, char8_t* const end, int64_t value)
{
    return concat_int(ptr, end, value);
}

char8_t* concat(char8_t* ptr, char8_t* const end, uint32_t value)
{
    return concat_int(ptr, end, value);
}

char8_t* concat(char8_t* ptr, char8_t* const end, uint64_t value)
{
    return concat_int(ptr, end, value);
}

//  UnTech::StringBuilder::hex<0> has no padding
char8_t* concat(char8_t* ptr, char8_t* const end, const hex<0> value)
{
    static_assert(std::is_same_v<decltype(value.value), const uint32_t>);

    const auto r = std::to_chars(reinterpret_cast<char*>(ptr), reinterpret_cast<char*>(end), value.value, 16);
    assert(r.ec == std::errc());

    return reinterpret_cast<char8_t*>(r.ptr);
}

// UnTech::StringBuilder::hex<N> has padding
template <unsigned N>
char8_t* concat(char8_t* buffer, char8_t* const end, const hex<N> value)
{
    static_assert(std::is_same_v<decltype(value.value), const uint32_t>);

    constexpr size_t bSize = N;
    static_assert(bSize == stringSize(hex<N>(uint32_t(0))));

    char8_t* ptr = buffer + bSize;

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
template char8_t* concat(char8_t*, char8_t* const, const hex<4>);
template char8_t* concat(char8_t*, char8_t* const, const hex<6>);
template char8_t* concat(char8_t*, char8_t* const, const hex<8>);

}
