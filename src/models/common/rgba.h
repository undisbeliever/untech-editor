#pragma once

#include <cstdint>

namespace UnTech {

union rgba {
    uint32_t value;
    struct {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t alpha;
    };

    rgba(uint32_t v)
        : value(v)
    {
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
        return value & 0xFFFFFF;
    }

    inline bool operator==(const rgba& o) const
    {
        return value == o.value;
    }

    inline bool operator!=(const rgba& o) const
    {
        return value != o.value;
    }

    inline bool operator<(const rgba& o) const
    {
        return value < o.value;
    }
};
}
