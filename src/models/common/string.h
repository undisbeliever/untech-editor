/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <climits>
#include <cstdint>
#include <optional>
#include <string>

namespace UnTech::String {

/**
 * @return true if str is a NULL terminated utf8 well formed string.
 */
// Cannot use std::u8string_view, code expects a null terminated string
bool checkUtf8WellFormed(const std::u8string& str);

// returns true if str ends in cmp
static inline bool endsWith(const std::u8string_view str, const std::u8string_view cmp)
{
    if (str.size() < cmp.size()) {
        return false;
    }
    return std::equal(cmp.rbegin(), cmp.rend(), str.rbegin());
}

static inline std::u8string& ltrim(std::u8string& s)
{
    size_t f = s.find_first_not_of(u8" \t\n\r", 0, 4);
    s.erase(0, f);
    return s;
}

static inline std::u8string& rtrim(std::u8string& s)
{
    size_t f = s.find_last_not_of(u8" \t\n\r", s.npos, 4);
    s.erase(0, f);
    return s;
}

// trim from both ends
static inline std::u8string& trim(std::u8string& s)
{
    return ltrim(rtrim(s));
}

std::optional<int32_t> toInt32(const std::u8string_view s);
std::optional<uint32_t> toUint32(const std::u8string_view s);
std::optional<uint16_t> toUint16(const std::u8string_view s);
std::optional<uint8_t> toUint8(const std::u8string_view s);

std::optional<uint32_t> hexToUint32(const std::u8string_view s);

std::optional<int32_t> decimalOrHexToInt32(const std::u8string_view s);
std::optional<uint32_t> decimalOrHexToUint32(const std::u8string_view s);
std::optional<uint16_t> decimalOrHexToUint16(const std::u8string_view s);

}
