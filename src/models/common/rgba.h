/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <cstdint>

namespace UnTech {

union rgba {
    struct {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t alpha;
    };
    uint32_t _value;

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
    rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
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
        return _value == o._value;
    }

    inline bool operator!=(const rgba& o) const
    {
        return _value != o._value;
    }

    inline bool operator<(const rgba& o) const
    {
        return _value < o._value;
    }
};
}
