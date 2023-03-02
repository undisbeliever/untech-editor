/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

// NOLINTNEXTLINE(modernize-deprecated-headers)
#include "string.h"

#include <cassert>
#include <fstream>
#include <span>
#include <string>

namespace UnTech::String {

// Returns number of bytes in the code point
// Return std::nullopt if `c` is not well formed.
static __attribute__((always_inline)) inline std::optional<unsigned> checkUtf8Codepoint(std::span<const char8_t, 4> c)
{
    static_assert(sizeof(char8_t) == 1);
    static_assert(sizeof(char8_t) == sizeof(char));

    // Code based off The Unicode 7.0 Standard
    // Table 3-7. *Well-Formed UTF-8 Byte Sequences*

    // NOLINTBEGIN(bugprone-branch-clone)

    if (c[0] <= 0x7F) {
        return 1;
    }
    else if ((c[0] >= 0xC2 && c[0] <= 0xDF)
             && (c[1] >= 0x80 && c[1] <= 0xBF)) {
        return 2;
    }
    else if (c[0] == 0xE0
             && (c[1] >= 0xA0 && c[1] <= 0xBF)
             && (c[2] >= 0x80 && c[2] <= 0xBF)) {
        return 3;
    }
    else if ((c[0] >= 0xE1 && c[0] <= 0xEC)
             && (c[1] >= 0x80 && c[1] <= 0xBF)
             && (c[2] >= 0x80 && c[2] <= 0xBF)) {
        return 3;
    }
    else if (c[0] == 0xED
             && (c[1] >= 0x80 && c[1] <= 0x9F)
             && (c[2] >= 0x80 && c[2] <= 0xBF)) {
        return 3;
    }
    else if ((c[0] >= 0xEE && c[0] <= 0xEF)
             && (c[1] >= 0x80 && c[1] <= 0xBF)
             && (c[2] >= 0x80 && c[2] <= 0xBF)) {
        return 3;
    }
    else if (c[0] == 0xF0
             && (c[1] >= 0x90 && c[1] <= 0xBF)
             && (c[2] >= 0x80 && c[2] <= 0xBF)
             && (c[3] >= 0x80 && c[3] <= 0xBF)) {
        return 4;
    }
    else if ((c[0] >= 0xF1 && c[0] <= 0xF3)
             && (c[1] >= 0x80 && c[1] <= 0xBF)
             && (c[2] >= 0x80 && c[2] <= 0xBF)
             && (c[3] >= 0x80 && c[3] <= 0xBF)) {
        return 4;
    }
    else if (c[0] == 0xF4
             && (c[1] >= 0x80 && c[1] <= 0x8F)
             && (c[2] >= 0x80 && c[2] <= 0xBF)
             && (c[3] >= 0x80 && c[3] <= 0xBF)) {
        return 4;
    }
    else {
        return std::nullopt;
    }

    // NOLINTEND(bugprone-branch-clone)
}

bool checkUtf8WellFormed(const std::u8string_view str)
{
    std::span<const char8_t> span = str;

    while (span.size() >= 4) {
        const auto l = checkUtf8Codepoint(span.first<4>());
        if (!l) {
            return false;
        }
        span = span.subspan(l.value());
    }

    while (!span.empty()) {
        std::array<char8_t, 4> array{};
        array.fill(0);

        assert(span.size() < array.size());
        std::copy(span.begin(), span.end(), array.begin());

        const auto l = checkUtf8Codepoint(array);
        if (!l || l > span.size()) {
            return false;
        }
        span = span.subspan(l.value());
    }

    return true;
}

static inline std::optional<uint32_t> toUint32_limited(const std::u8string_view s, const uint32_t max)
{
    if (s.empty()) {
        return std::nullopt;
    }

    uint64_t i = 0;

    for (const char8_t c : s) {
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

std::optional<uint32_t> toUint32(const std::u8string_view s)
{
    return toUint32_limited(s, UINT32_MAX);
}

std::optional<uint16_t> toUint16(const std::u8string_view s)
{
    if (auto v = toUint32_limited(s, UINT16_MAX)) {
        return *v;
    }
    return std::nullopt;
}

std::optional<uint8_t> toUint8(const std::u8string_view s)
{
    if (auto v = toUint32_limited(s, UINT8_MAX)) {
        return *v;
    }
    return std::nullopt;
}

std::optional<uint32_t> hexToUint32(const std::u8string_view s)
{
    if (s.empty() || s.size() > 8) {
        return std::nullopt;
    }

    uint32_t i = 0;

    for (const char8_t c : s) {
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

std::optional<uint32_t> decimalOrHexToUint32(const std::u8string_view s)
{
    if (s.starts_with(u8"0x") || s.starts_with(u8"0X")) {
        return hexToUint32(s.substr(2));
    }
    else {
        return toUint32(s);
    }
}

std::optional<uint16_t> decimalOrHexToUint16(const std::u8string_view s)
{
    if (s.starts_with(u8"0x") || s.starts_with(u8"0X")) {
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

std::optional<int32_t> toInt32(const std::u8string_view s)
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

std::optional<int32_t> decimalOrHexToInt32(const std::u8string_view s)
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
