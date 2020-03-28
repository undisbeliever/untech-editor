/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

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
        int px = int(p.x) - int(offset.x);
        int py = int(p.y) - int(offset.y);

        if (!int_ms8_t::isValid(px)) {
            throw std::out_of_range("upoint.x");
        }
        if (!int_ms8_t::isValid(py)) {
            throw std::out_of_range("upoint.y");
        }

        return ms8point(px, py);
    }

    ms8point flip(bool hFlip, bool vFlip) const
    {
        ms8point p = *this;

        if (hFlip) {
            p.x = -p.x;
        }
        if (vFlip) {
            p.y = -p.y;
        }

        return p;
    }

    ms8point flip(bool hFlip, bool vFlip, unsigned width, unsigned height) const
    {
        ms8point p = *this;

        if (hFlip) {
            p.x = -p.x - width;
        }
        if (vFlip) {
            p.y = -p.y - height;
        }

        return p;
    }

    ms8point flip(bool hFlip, bool vFlip, unsigned squareSize) const
    {
        return flip(hFlip, vFlip, squareSize, squareSize);
    }

    bool operator==(const ms8point& o) const
    {
        return x == o.x && y == o.y;
    }

    bool operator!=(const ms8point& o) const
    {
        return x != o.x || y != o.y;
    }

    bool operator==(const point& o) const
    {
        return x == o.x && y == o.y;
    }

    bool operator!=(const point& o) const
    {
        return x != o.x || y != o.y;
    }
};

inline bool operator==(const point& a, const ms8point& b)
{
    return b == a;
}
inline bool operator!=(const point& a, const ms8point& b)
{
    return b != a;
}

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

    ms8rect(const ms8point& p, uint8_t squareSize)
        : x(p.x)
        , y(p.y)
        , width(squareSize)
        , height(squareSize)
    {
    }

    static inline ms8rect createFromOffset(const urect& r, const upoint& offset)
    {
        int rx = int(r.x) - int(offset.x);
        int ry = int(r.y) - int(offset.y);

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

    // extends this m8rect so the other ms8rect fits in it.
    void extend(const ms8rect& other)
    {
        int oldRight = this->right();
        int oldBottom = this->bottom();

        if (other.x < this->x) {
            this->x = other.x;
        }
        if (other.y < this->y) {
            this->y = other.y;
        }
        this->width = std::max(oldRight, other.right()) - this->x;
        this->height = std::max(oldBottom, other.bottom()) - this->y;
    }

    inline bool contains(const ms8point& p) const
    {
        return p.x >= left() && p.x < right()
               && p.y >= top() && p.y < bottom();
    }

    inline bool contains(const point& p) const
    {
        return p.x >= left() && p.x < right()
               && p.y >= top() && p.y < bottom();
    }

    ms8rect flip(bool hFlip, bool vFlip) const
    {
        ms8rect r = *this;

        if (hFlip) {
            r.x = -r.x - r.width;
        }
        if (vFlip) {
            r.y = -r.y - r.height;
        }

        return r;
    }

    bool operator==(const ms8rect& o) const
    {
        return x == o.x && y == o.y && width == o.width && height == o.height;
    }

    bool operator!=(const ms8rect& o) const
    {
        return x != o.x || y != o.y || width != o.width || height != o.height;
    }

    bool operator==(const rect& o) const
    {
        return x == o.x && y == o.y && width == o.width && height == o.height;
    }

    bool operator!=(const rect& o) const
    {
        return x != o.x || y != o.y || width != o.width || height != o.height;
    }
};

inline bool operator==(const rect& a, const ms8rect& b)
{
    return b == a;
}
inline bool operator!=(const rect& a, const ms8rect& b)
{
    return b != a;
}
}
