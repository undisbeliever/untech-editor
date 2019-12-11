/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "idstring.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>

namespace UnTech {
namespace StringBuilder {

// Disable automated type casting on stringBuilder
template <typename T>
size_t stringSize(const std::string& s) = delete;

inline size_t stringSize(const std::string& s)
{
    return s.size();
}

inline size_t stringSize(const idstring& id)
{
    return id.str().size();
}

inline size_t stringSize(const char* s)
{
    return strlen(s);
}

inline constexpr size_t stringSize(int32_t)
{
    return 12;
}

inline constexpr size_t stringSize(int64_t)
{
    return 21;
}

inline constexpr size_t stringSize(uint32_t)
{
    return 11;
}

inline constexpr size_t stringSize(uint64_t)
{
    return 21;
}

// Disable automated type casting on stringBuilder
template <typename T>
void concat(std::string&, T) = delete;

inline void concat(std::string& str, const std::string& source)
{
    str.append(source);
}

inline void concat(std::string& str, const idstring& source)
{
    str.append(source);
}

inline void concat(std::string& str, const char* c_str)
{
    str.append(c_str);
}

void concat(std::string& str, int32_t value);
void concat(std::string& str, int64_t value);
void concat(std::string& str, uint32_t value);
void concat(std::string& str, uint64_t value);

}

template <typename... Args>
inline std::string stringBuilder(const Args&... args)
{
    const size_t size = (... + StringBuilder::stringSize(args)) + 1;
    std::string str;
    str.reserve(size);

    ((StringBuilder::concat(str, args)), ...);

    return str;
}

inline std::string&& stringBuilder(std::string&& str)
{
    return std::move(str);
}

inline const std::string& stringBuilder(const std::string& str)
{
    return str;
}

inline std::string stringBuilder(const char* c_str)
{
    return std::string(c_str);
}

}
