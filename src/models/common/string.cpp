/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "string.h"
#include <cassert>
#include <fstream>
#include <string>

namespace UnTech::String {

// Cannot use std::string_view, code expects a null terminated string
bool checkUtf8WellFormed(const std::string& str)
{
    // ::TODO change code to accept std::string_view
    // ::TODO test this code works with a std::string_view substring

    const unsigned char* c = (const unsigned char*)str.c_str();
    size_t length = 0;

    if (str.empty()) {
        return true;
    }

    while (*c) {
        // Code based off The Unicode 7.0 Standard
        // Table 3-7. *Well-Formed UTF-8 Byte Sequences*

        if (c[0] <= 0x7F) {
            c++;
            length++;
        }
        else if ((c[0] >= 0xC2 && c[0] <= 0xDF)
                 && (c[1] >= 0x80 && c[1] <= 0xBF)) {
            c += 2;
            length += 2;
        }
        else if (c[0] == 0xE0
                 && (c[1] >= 0xA0 && c[1] <= 0xBF)
                 && (c[2] >= 0x80 && c[2] <= 0xBF)) {
            c += 3;
            length += 3;
        }
        else if ((c[0] >= 0xE1 && c[0] <= 0xEC)
                 && (c[1] >= 0x80 && c[1] <= 0xBF)
                 && (c[2] >= 0x80 && c[2] <= 0xBF)) {
            c += 3;
            length += 3;
        }
        else if (c[0] == 0xED
                 && (c[1] >= 0x80 && c[1] <= 0x9F)
                 && (c[2] >= 0x80 && c[2] <= 0xBF)) {
            c += 3;
            length += 3;
        }
        else if ((c[0] >= 0xEE && c[0] <= 0xEF)
                 && (c[1] >= 0x80 && c[1] <= 0xBF)
                 && (c[2] >= 0x80 && c[2] <= 0xBF)) {
            c += 3;
            length += 3;
        }
        else if (c[0] == 0xF0
                 && (c[1] >= 0x90 && c[1] <= 0xBF)
                 && (c[2] >= 0x80 && c[2] <= 0xBF)
                 && (c[3] >= 0x80 && c[3] <= 0xBF)) {
            c += 4;
            length += 4;
        }
        else if ((c[0] >= 0xF1 && c[0] <= 0xF3)
                 && (c[1] >= 0x80 && c[1] <= 0xBF)
                 && (c[2] >= 0x80 && c[2] <= 0xBF)
                 && (c[3] >= 0x80 && c[3] <= 0xBF)) {
            c += 4;
            length += 4;
        }
        else if (c[0] == 0xF4
                 && (c[1] >= 0x80 && c[1] <= 0x8F)
                 && (c[2] >= 0x80 && c[2] <= 0xBF)
                 && (c[3] >= 0x80 && c[3] <= 0xBF)) {
            c += 4;
            length += 4;
        }
        else {
            return false;
        }
    }

    return length == str.size();
}

static inline std::optional<uint32_t> toUint32_limited(const std::string_view s, const uint32_t max)
{
    if (s.empty()) {
        return std::nullopt;
    }

    uint64_t i = 0;

    for (const char c : s) {
        if (c < '0' || c > '9') {
            return std::nullopt;
        }

        i *= 10;
        i += c - '0';

        if (i > max) {
            return std::nullopt;
        }
    }

    return i;
}

std::optional<uint32_t> toUint32(const std::string_view s)
{
    return toUint32_limited(s, UINT32_MAX);
}

std::optional<uint16_t> toUint16(const std::string_view s)
{
    if (auto v = toUint32_limited(s, UINT16_MAX)) {
        return *v;
    }
    return std::nullopt;
}

std::optional<uint8_t> toUint8(const std::string_view s)
{
    if (auto v = toUint32_limited(s, UINT8_MAX)) {
        return *v;
    }
    return std::nullopt;
}

std::optional<uint32_t> hexToUint32(const std::string_view s)
{
    if (s.empty() || s.size() > 8) {
        return std::nullopt;
    }

    uint32_t i = 0;

    for (const char c : s) {
        i <<= 4;
        if (c >= u8'0' && c <= u8'9') {
            i |= c - u8'0';
        }
        else if (c >= u8'A' && c <= u8'F') {
            i |= c - u8'A' + 10;
        }
        else if (c >= u8'a' && c <= u8'f') {
            i |= c - u8'a' + 10;
        }
        else {
            return std::nullopt;
        }
    }

    return i;
}

std::optional<uint32_t> decimalOrHexToUint32(const std::string_view s)
{
    if (s.starts_with("0x") || s.starts_with("0X")) {
        return hexToUint32(s.substr(2));
    }
    else {
        return toUint32(s);
    }
}

std::optional<uint16_t> decimalOrHexToUint16(const std::string_view s)
{
    if (s.starts_with("0x") || s.starts_with("0X")) {
        if (auto value = hexToUint32(s.substr(2))) {
            if (*value <= UINT16_MAX) {
                return *value;
            }
        }
    }
    else {
        return toUint16(s);
    }

    return std::nullopt;
}

std::optional<int32_t> toInt32(const std::string_view s)
{
    static_assert(INT32_MIN < 0);
    constexpr uint64_t negative_min = -int64_t(INT32_MIN);
    static_assert(negative_min < UINT32_MAX);

    if (s.empty()) {
        return std::nullopt;
    }

    if (s.front() != '-') {
        if (auto v = toUint32_limited(s, INT32_MAX)) {
            return *v;
        }
    }
    else {
        if (const auto v = toUint32_limited(s.substr(1), negative_min)) {
            return -int64_t(*v);
        }
    }

    return std::nullopt;
}

std::optional<int32_t> decimalOrHexToInt32(const std::string_view s)
{
    static_assert(INT32_MIN < 0);
    constexpr uint64_t negative_min = -int64_t(INT32_MIN);
    static_assert(negative_min < UINT32_MAX);

    if (s.empty()) {
        return std::nullopt;
    }

    if (s.front() != '-') {
        if (auto v = decimalOrHexToUint32(s.substr(1))) {
            if (*v < INT32_MAX) {
                return *v;
            }
        }
    }
    else {
        if (auto v = decimalOrHexToUint32(s.substr(1))) {
            if (*v < negative_min) {
                return -int64_t(*v);
            }
        }
    }

    return std::nullopt;
}

}
