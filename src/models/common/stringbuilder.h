/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "idstring.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <string>

namespace UnTech::StringBuilder {

// StringBuilder placeholder for a hex string with exactly N digits
// If N is zero there will be no padding
template <unsigned N>
struct hex {
    static_assert(N <= 8);

    const uint32_t value;

    constexpr explicit hex(uint32_t v)
        : value(v)
    {
        constexpr uint32_t maxValue = N > 0 ? (uint64_t(1) << (N * 4)) - 1
                                            : UINT32_MAX;
        assert(value <= maxValue);
    }

    constexpr explicit hex(uint16_t v) requires(N == 4)
        : value(v)
    {
    }

    // Only allow `uint32_t` (or maybe `uint16_t`) to be converted to a hex string
    template <class T>
    hex(T) = delete;
};

// Disable automated type casting on stringBuilder
template <typename T>
size_t stringSize(T s) = delete;

inline size_t stringSize(const std::u8string& s) { return s.size(); }
inline size_t stringSize(const idstring& id) { return id.str().size(); }
inline constexpr size_t stringSize(const std::u8string_view s) { return s.size(); }
inline constexpr size_t stringSize(int32_t) { return 12; }
inline constexpr size_t stringSize(int64_t) { return 21; }
inline constexpr size_t stringSize(uint32_t) { return 11; }
inline constexpr size_t stringSize(uint64_t) { return 21; }

template <unsigned N>
inline constexpr size_t stringSize(const hex<N>) { return N; }
inline constexpr size_t stringSize(const hex<0>) { return 8; } // cppcheck-suppress syntaxError

// Disable automated type casting on stringBuilder
template <typename T>
void concat(char8_t* ptr, char8_t* const end, T) = delete;

inline char8_t* concat(char8_t* ptr, char8_t* const, const std::u8string& source)
{
    return std::copy(source.begin(), source.end(), ptr);
}

inline char8_t* concat(char8_t* ptr, char8_t* const, const std::u8string_view source)
{
    return std::copy(source.begin(), source.end(), ptr);
}

inline char8_t* concat(char8_t* ptr, char8_t* const, const idstring& source)
{
    return std::copy(source.str().begin(), source.str().end(), ptr);
}

inline char8_t* concat(char8_t* ptr, char8_t* const end, const char8_t* c_str)
{
    return concat(ptr, end, std::u8string_view(c_str));
}

char8_t* concat(char8_t* ptr, char8_t* const end, int32_t value);
char8_t* concat(char8_t* ptr, char8_t* const end, int64_t value);
char8_t* concat(char8_t* ptr, char8_t* const end, uint32_t value);
char8_t* concat(char8_t* ptr, char8_t* const end, uint64_t value);

template <unsigned N>
char8_t* concat(char8_t* ptr, char8_t* const end, const hex<N> value);
char8_t* concat(char8_t* ptr, char8_t* const end, const hex<0> value);

// Template magic to convert `const char8_t*` to std::u8string_view.
inline std::u8string_view convert(const char8_t* a) { return a; }
template <typename T>
inline const T& convert(const T& a) { return a; }

template <typename... Args>
inline std::u8string buildString(const Args&... args)
{
    const size_t estimatedSize = (... + UnTech::StringBuilder::stringSize(args)) + 1;

    std::u8string str(estimatedSize, '\0');

    // Using `char8_t*` here as `std::to_chars()` uses pointers.
    char8_t* ptr = str.data();
    char8_t* const end = str.data() + str.size();

    auto process = [&](const auto& a) {
        ptr = UnTech::StringBuilder::concat(ptr, end, a);
    };
    (process(args), ...);

    assert(ptr >= str.data() && ptr < end);
    const size_t newSize = std::distance(str.data(), ptr);
    assert(newSize < estimatedSize);

    str.erase(str.begin() + newSize, str.end());

    return str;
}

}

namespace UnTech {

using hex = StringBuilder::hex<0>;
using hex_4 = StringBuilder::hex<4>;
using hex_6 = StringBuilder::hex<6>;
using hex_8 = StringBuilder::hex<8>;

template <typename... Args>
inline std::u8string stringBuilder(const Args&... args)
{
    return UnTech::StringBuilder::buildString(StringBuilder::convert(args)...);
}

inline std::u8string&& stringBuilder(std::u8string&& str)
{
    return std::move(str);
}

inline const std::u8string& stringBuilder(const std::u8string& str)
{
    return str;
}

inline std::u8string stringBuilder(const std::u8string_view str)
{
    return std::u8string(str);
}

inline std::u8string stringBuilder(const char8_t* c_str)
{
    return std::u8string(c_str);
}

}
