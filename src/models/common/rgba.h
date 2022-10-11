/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <cstdint>
#include <string>
#include <tuple>

namespace UnTech {

struct alignas(4) rgba {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;

    static constexpr rgba fromRgba(uint32_t rgbaValue)
    {
        return {
            uint8_t((rgbaValue >> 0) & 0xff),
            uint8_t((rgbaValue >> 8) & 0xff),
            uint8_t((rgbaValue >> 16) & 0xff),
            uint8_t((rgbaValue >> 24) & 0xff)
        };
    }

    static constexpr rgba fromRgbHex(uint32_t rgbHex)
    {
        return {
            uint8_t((rgbHex >> 16) & 0xff),
            uint8_t((rgbHex >> 8) & 0xff),
            uint8_t((rgbHex >> 0) & 0xff)
        };
    }

    static constexpr rgba fromRgbaHex(uint32_t rgbaHex)
    {
        return {
            uint8_t((rgbaHex >> 24) & 0xff),
            uint8_t((rgbaHex >> 16) & 0xff),
            uint8_t((rgbaHex >> 8) & 0xff),
            uint8_t((rgbaHex >> 0) & 0xff)
        };
    }

    constexpr rgba()
        : red(0)
        , green(0)
        , blue(0)
        , alpha(0)
    {
    }

    constexpr rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : red(r)
        , green(g)
        , blue(b)
        , alpha(a)
    {
    }

    inline uint32_t rgb() const
    {
        return (blue << 16) | (green << 8) | red;
    }

    inline uint32_t rgbaValue() const
    {
        return (alpha << 24) | (blue << 16) | (green << 8) | red;
    }

    inline uint32_t rgbHex() const
    {
        return (red << 16) | (green << 8) | blue;
    }

    inline uint32_t rgbaHex() const
    {
        return (alpha << 24) | (red << 16) | (green << 8) | blue;
    }

    std::u8string rgbHexString() const;

    bool operator==(const rgba&) const = default;

    // Cannot default: Comparison preformed in reverse order
    bool operator<(const rgba& o) const
    {
        return std::tie(alpha, blue, green, red) < std::tie(o.alpha, o.blue, o.green, o.red);
    }
};
}
