/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <cstdint>
#include <tuple>

namespace UnTech {

struct alignas(4) rgba {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;

    static rgba fromRgba(uint32_t rgbaValue)
    {
        return rgba((rgbaValue >> 0) & 0xff,
                    (rgbaValue >> 8) & 0xff,
                    (rgbaValue >> 16) & 0xff,
                    (rgbaValue >> 24) & 0xff);
    }

    static rgba fromRgbHex(uint32_t rgbHex)
    {
        return rgba((rgbHex >> 16) & 0xff,
                    (rgbHex >> 8) & 0xff,
                    (rgbHex >> 0) & 0xff);
    }

    static rgba fromRgbaHex(uint32_t rgbaHex)
    {
        return rgba((rgbaHex >> 24) & 0xff,
                    (rgbaHex >> 16) & 0xff,
                    (rgbaHex >> 8) & 0xff,
                    (rgbaHex >> 0) & 0xff);
    }

    rgba() = default;
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

    inline bool operator==(const rgba& o) const
    {
        return red == o.red
               && green == o.green
               && blue == o.blue
               && alpha == o.alpha;
    }

    inline bool operator!=(const rgba& o) const
    {
        return !(*this == o);
    }

    inline bool operator<(const rgba& o) const
    {
        return std::tie(alpha, blue, green, red) < std::tie(o.alpha, o.blue, o.green, o.red);
    }
};
}
