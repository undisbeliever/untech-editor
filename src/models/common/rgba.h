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
        return rgba(rgbaValue & 0xff,
                    (rgbaValue >> 8) & 0xff,
                    (rgbaValue >> 16) & 0xff,
                    (rgbaValue >> 24) & 0xff);
    }

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
