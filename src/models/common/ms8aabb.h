#ifndef _UNTECH_MODELS_COMMON_MS8AABB_H_
#define _UNTECH_MODELS_COMMON_MS8AABB_H_

#include "aabb.h"
#include "int_ms8_t.h"
#include <algorithm>
#include <stdexcept>

namespace UnTech {

struct ms8point {
    int_ms8_t x;
    int_ms8_t y;

    ms8point()
        : x(0)
        , y(0)
    {
    }

    ms8point(int_ms8_t x, int_ms8_t y)
        : x(x)
        , y(y)
    {
    }

    static inline ms8point createFromOffset(const upoint& p, const upoint& offset)
    {
        int px = p.x - offset.x;
        int py = p.y - offset.y;

        if (!int_ms8_t::isValid(px)) {
            throw std::out_of_range("upoint.x");
        }
        if (!int_ms8_t::isValid(py)) {
            throw std::out_of_range("upoint.y");
        }

        return ms8point(px, py);
    }

    bool operator==(const ms8point& o) const
    {
        return x == o.x && y == o.y;
    }

    bool operator!=(const ms8point& o) const
    {
        return x != o.x || y != o.y;
    }
};

struct ms8rect {
    int_ms8_t x;
    int_ms8_t y;
    uint8_t width;
    uint8_t height;

    ms8rect()
        : x(0)
        , y(0)
        , width(0)
        , height(0)
    {
    }

    ms8rect(int x, int y, uint8_t width, uint8_t height)
        : x(x)
        , y(y)
        , width(width)
        , height(height)
    {
    }

    static inline ms8rect createFromOffset(const urect& r, const upoint& offset)
    {
        int rx = r.x - offset.x;
        int ry = r.y - offset.y;

        if (!int_ms8_t::isValid(rx)) {
            throw std::out_of_range("urect.x");
        }
        if (!int_ms8_t::isValid(ry)) {
            throw std::out_of_range("urect.y");
        }
        if (r.width > UINT8_MAX) {
            throw std::out_of_range("urect.width");
        }
        if (r.height > UINT8_MAX) {
            throw std::out_of_range("urect.height");
        }

        return ms8rect(rx, ry, r.width, r.height);
    }

    inline int left() const { return x; }
    inline int right() const { return x + width; }
    inline int top() const { return y; }
    inline int bottom() const { return y + height; }

    inline usize size() const { return usize(width, height); }

    inline bool contains(const ms8point& p) const
    {
        return p.x >= left() && p.x < right()
               && p.y >= top() && p.y < bottom();
    }

    bool operator==(const ms8rect& o) const
    {
        return x == o.x && y == o.y && width == o.width && height == o.height;
    }

    bool operator!=(const ms8rect& o) const
    {
        return x != o.x || y != o.y || width != o.width || height != o.height;
    }
};
}
#endif
