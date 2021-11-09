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

// Disable automated type casting on stringBuilder
template <typename T>
size_t stringSize(T s) = delete;

inline size_t stringSize(const std::string& s) { return s.size(); }
inline size_t stringSize(const idstring& id) { return id.str().size(); }
inline constexpr size_t stringSize(const std::string_view s) { return s.size(); }
inline constexpr size_t stringSize(int32_t) { return 12; }
inline constexpr size_t stringSize(int64_t) { return 21; }
inline constexpr size_t stringSize(uint32_t) { return 11; }
inline constexpr size_t stringSize(uint64_t) { return 21; }

// Disable automated type casting on stringBuilder
template <typename T>
void concat(char* ptr, char* const end, T) = delete;

inline char* concat(char* ptr, char* const, const std::string& source)
{
    return std::copy(source.begin(), source.end(), ptr);
}

inline char* concat(char* ptr, char* const, const std::string_view source)
{
    return std::copy(source.begin(), source.end(), ptr);
}

inline char* concat(char* ptr, char* const, const idstring& source)
{
    return std::copy(source.str().begin(), source.str().end(), ptr);
}

inline char* concat(char* ptr, char* const end, const char* c_str)
{
    return concat(ptr, end, std::string_view(c_str));
}

char* concat(char* ptr, char* const end, int32_t value);
char* concat(char* ptr, char* const end, int64_t value);
char* concat(char* ptr, char* const end, uint32_t value);
char* concat(char* ptr, char* const end, uint64_t value);

// Template magic to convert `const char*` to std::string_view.
inline std::string_view convert(const char* a) { return a; }
template <typename T>
inline const T& convert(const T& a) { return a; }

template <typename... Args>
inline std::string buildString(const Args&... args)
{
    const size_t size = (... + UnTech::StringBuilder::stringSize(args)) + 1;

    std::string str(size, '\0');

    // Using `char*` here as `std::to_chars()` uses pointers.
    char* ptr = str.data();
    char* const end = str.data() + str.size();

    auto process = [&](const auto& a) {
        ptr = UnTech::StringBuilder::concat(ptr, end, a);
    };
    (process(args), ...);

    assert(ptr >= str.data() && ptr < end);
    const auto newSize = std::distance(str.data(), ptr);

    str.erase(str.begin() + newSize, str.end());

    return str;
}

}

namespace UnTech {

template <typename... Args>
inline std::string stringBuilder(const Args&... args)
{
    return UnTech::StringBuilder::buildString(StringBuilder::convert(args)...);
}

inline std::string&& stringBuilder(std::string&& str)
{
    return std::move(str);
}

inline const std::string& stringBuilder(const std::string& str)
{
    return str;
}

inline std::string stringBuilder(const std::string_view str)
{
    return std::string(str);
}

inline std::string stringBuilder(const char* c_str)
{
    return std::string(c_str);
}

}
