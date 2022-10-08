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

inline size_t stringSize(const idstring& id) { return id.str().size(); }
inline constexpr size_t stringSize(const std::u8string& s) { return s.size(); }
inline constexpr size_t stringSize(const std::u8string_view s) { return s.size(); }
inline constexpr size_t stringSize(const char8_t* s) { return std::u8string_view(s).size(); }

inline constexpr size_t stringSize(int32_t) { return 12; }
inline constexpr size_t stringSize(int64_t) { return 21; }
inline constexpr size_t stringSize(uint32_t) { return 11; }
inline constexpr size_t stringSize(uint64_t) { return 21; }

template <unsigned N>
inline constexpr size_t stringSize(const hex<N>) { return N; }
inline constexpr size_t stringSize(const hex<0>) { return 8; } // cppcheck-suppress syntaxError

// Disable automated type casting on stringBuilder
template <typename T>
void concat(std::u8string&, T) = delete;

inline void concat(std::u8string& s, const idstring& v) { s.append(v.str()); }
inline void concat(std::u8string& s, const std::u8string& v) { s.append(v); }
inline void concat(std::u8string& s, const std::u8string_view v) { s.append(v); }
inline void concat(std::u8string& s, const char8_t* v) { s.append(std::u8string_view(v)); }

void concat(std::u8string& s, int32_t v);
void concat(std::u8string& s, int64_t v);
void concat(std::u8string& s, uint32_t v);
void concat(std::u8string& s, uint64_t v);

template <unsigned N>
void concat(std::u8string& s, const hex<N> value);
void concat(std::u8string& s, const hex<0> value);

}

namespace UnTech {

using hex = StringBuilder::hex<0>;
using hex_4 = StringBuilder::hex<4>;
using hex_6 = StringBuilder::hex<6>;
using hex_8 = StringBuilder::hex<8>;

template <typename... Args>
inline std::u8string stringBuilder(const Args&... args)
{
    const size_t estimatedSize = (... + UnTech::StringBuilder::stringSize(args)) + 1;

    std::u8string str;
    str.reserve(estimatedSize);

    (UnTech::StringBuilder::concat(str, args), ...);

    return str;
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
