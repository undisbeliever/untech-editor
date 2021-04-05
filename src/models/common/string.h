/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "optional.h"
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <string>

namespace UnTech::String {

/**
 * @return true if str is a NULL terminated utf8 well formed string.
 */
// Cannot use std::string_view, code expects a null terminated string
bool checkUtf8WellFormed(const std::string& str);

// returns true if str ends in cmp
static inline bool endsWith(const std::string_view str, const std::string_view cmp)
{
    if (str.size() < cmp.size()) {
        return false;
    }
    return std::equal(cmp.rbegin(), cmp.rend(), str.rbegin());
}

static inline std::string& ltrim(std::string& s)
{
    size_t f = s.find_first_not_of(" \t\n\r", 0, 4);
    s.erase(0, f);
    return s;
}

static inline std::string& rtrim(std::string& s)
{
    size_t f = s.find_last_not_of(" \t\n\r", s.npos, 4);
    s.erase(0, f);
    return s;
}

// trim from both ends
static inline std::string& trim(std::string& s)
{
    return ltrim(rtrim(s));
}

/* Convert a string to an integer.
 * String may be encased in spaces.
 */
static inline optional<int> toInt(const std::string_view s)
{
    using namespace std::literals;

    const char* cstr = s.data();
    const char* last = cstr + s.find_last_not_of(" \t\n\r"sv, s.npos);
    char* parseEnd;

    int ret = strtol(cstr, &parseEnd, 0);

    if (parseEnd == cstr || parseEnd != last + 1) {
        optional<int>();
    }
    return ret;
}

/* Convert a string to an unsigned integer.
 * String may be encased in spaces.
 */
static inline optional<unsigned> toUnsigned(const std::string_view s)
{
    const char* cstr = s.data();
    const char* last = cstr + s.find_last_not_of(" \t\n\r", s.npos, 4);
    char* parseEnd;

    long ret = strtol(cstr, &parseEnd, 0);

    if (parseEnd == cstr || parseEnd != last + 1
        || ret < 0 || ret > long(UINT_MAX)) {

        optional<unsigned>();
    }
    return ret;
}

/* Convert a string to an uint8.
 * String may be encased in spaces.
 */
static inline optional<uint8_t> toUint8(const std::string_view s)
{
    const char* cstr = s.data();
    const char* last = cstr + s.find_last_not_of(" \t\n\r", s.npos, 4);
    char* parseEnd;

    long ret = strtol(cstr, &parseEnd, 0);

    if (parseEnd == cstr || parseEnd != last + 1
        || ret < 0 || ret > UINT8_MAX) {
        return optional<uint8_t>();
    }
    return (uint8_t)ret;
}

/* Convert a string to an uint16.
 * String may be encased in spaces.
 */
static inline optional<uint16_t> toUint16(const std::string_view s)
{
    const char* cstr = s.data();
    const char* last = cstr + s.find_last_not_of(" \t\n\r", s.npos, 4);
    char* parseEnd;

    long ret = strtol(cstr, &parseEnd, 0);

    if (parseEnd == cstr || parseEnd != last + 1
        || ret < 0 || ret > UINT16_MAX) {
        return optional<uint16_t>();
    }
    return (uint16_t)ret;
}

/* Convert a string to an integer.
 * Returns a default value if given invalid input.
 * String may be encased in spaces.
 */
static inline int toInt(const std::string_view s, int def)
{
    const char* cstr = s.data();
    const char* last = cstr + s.find_last_not_of(" \t\n\r", s.npos, 4);
    char* parseEnd;

    int ret = strtol(cstr, &parseEnd, 0);

    if (parseEnd == cstr || parseEnd != last + 1) {
        return def;
    }
    return ret;
}

static inline optional<long> toLong(const std::string_view s)
{
    const char* cstr = s.data();
    const char* last = cstr + s.find_last_not_of(" \t\n\r", s.npos, 4);
    char* parseEnd;

    long ret = strtol(cstr, &parseEnd, 0);

    if (parseEnd == cstr || parseEnd != last + 1) {
        return optional<long>();
    }
    return ret;
}

static inline long toLong(const std::string_view s, long def)
{
    const char* cstr = s.data();
    const char* last = cstr + s.find_last_not_of(" \t\n\r", s.npos, 4);
    char* parseEnd;

    long ret = strtol(cstr, &parseEnd, 0);

    if (parseEnd == cstr || parseEnd != last + 1) {
        return def;
    }
    return ret;
}

static inline optional<unsigned> hexToUnsigned(const std::string_view s)
{
    const char* cstr = s.data();
    const char* last = cstr + s.find_last_not_of(" \t\n\r", s.npos, 4);
    char* parseEnd;

    unsigned ret = strtoul(cstr, &parseEnd, 16);

    if (parseEnd == cstr || parseEnd != last + 1
        || ret > UINT_MAX) {
        return optional<unsigned>();
    }
    return ret;
}

}
