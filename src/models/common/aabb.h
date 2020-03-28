/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <algorithm>

namespace UnTech {

template <typename T>
struct _point;
template <typename T>
struct _rect;

typedef _point<int> point;
typedef _point<unsigned> upoint;
typedef _rect<int> rect;
typedef _rect<unsigned> urect;

template <typename T>
struct _point {
    T x;
    T y;

    _point()
        : x(0)
        , y(0)
    {
    }

    _point(T x, T y)
        : x(x)
        , y(y)
    {
    }

    bool operator==(const _point& o) const
    {
        return x == o.x && y == o.y;
    }

    bool operator!=(const _point& o) const
    {
        return x != o.x || y != o.y;
    }
};

struct usize {
    unsigned width;
    unsigned height;

    usize()
        : width(1)
        , height(1)
    {
    }

    usize(unsigned width, unsigned height)
        : width(width)
        , height(height)
    {
    }

    inline usize expand(const usize& s) const
    {
        return usize(std::max(width, s.width), std::max(height, s.height));
    }

    inline usize expand(const upoint& p) const
    {
        return usize(std::max(width, p.x), std::max(height, p.y));
    }

    inline usize expand(const urect& r) const;

    inline upoint clip(const upoint& p) const
    {
        return upoint(std::min(width, p.x), std::min(height, p.y));
    }

    inline bool contains(const upoint& p) const
    {
        return p.x < width && p.y < height;
    }

    inline bool contains(const upoint& p, unsigned squarePx) const
    {
        return p.x + squarePx <= width
               && p.y + squarePx <= height;
    }

    inline bool contains(const urect& r) const;

    bool operator==(const usize& o) const
    {
        return width == o.width && height == o.height;
    }

    bool operator!=(const usize& o) const
    {
        return width != o.width || height != o.height;
    }
};

template <typename T>
struct _rect {
    T x;
    T y;
    unsigned width;
    unsigned height;

    _rect()
        : x(0)
        , y(0)
        , width(1)
        , height(1)
    {
    }

    _rect(T x, T y, unsigned width, unsigned height)
        : x(x)
        , y(y)
        , width(width)
        , height(height)
    {
    }

    _rect(const _point<T>& point, unsigned size)
        : x(point.x)
        , y(point.y)
        , width(size)
        , height(size)
    {
    }

    _rect(const _point<T>& point, unsigned width, unsigned height)
        : x(point.x)
        , y(point.y)
        , width(width)
        , height(height)
    {
    }

    inline T left() const { return x; }
    inline T right() const { return x + width; }
    inline T top() const { return y; }
    inline T bottom() const { return y + height; }

    inline usize size() const { return { width, height }; }
    inline _point<T> bottomRight() const { return { bottom(), right() }; }

    inline bool contains(const _point<T>& p) const
    {
        return p.x >= left() && p.x < right()
               && p.y >= top() && p.y < bottom();
    }

    inline bool overlaps(const _rect& r) const
    {
        return left() < r.right() && right() > r.left()
               && top() < r.bottom() && bottom() > r.top();
    }

    inline bool overlaps(const _point<T>& p, unsigned squareSize) const
    {
        return left() < (p.x + squareSize) && right() > p.x
               && top() < (p.y + squareSize) && bottom() > p.y;
    }

    // Clips the _point within the width/height of the _rect
    inline _point<T> clipInside(const _point<T>& p) const
    {
        return _point<T>(std::min(p.x, width - 1), std::min(p.y, height - 1));
    }

    // Clips the _point within the width/height of the _rect
    _point<T> clipInside(const _point<T>& p, unsigned squareSize) const
    {
        _point<T> ret(0, 0);

        if (width > squareSize) {
            ret.x = std::min(p.x, width - squareSize);
        }
        if (height > squareSize) {
            ret.y = std::min(p.y, height - squareSize);
        }

        return ret;
    }

    // clips the rectangle inside this one
    // The method of clipping depends on the state of the previous rectangle.
    _rect clipInside(const _rect& r, const _rect& prev) const
    {
        _rect ret = r;

        if (ret.right() > width) {
            if (ret.width == prev.width) {
                // _rect was moved out of boundary
                ret.x = width - ret.width;
            }
            else {
                // _rect was resized out of boundary
                if (ret.x <= width) {
                    ret.width = width - ret.x;
                }
                else {
                    ret.x = width;
                    ret.width = width;
                }
            }
        }

        if (ret.bottom() > height) {
            if (ret.height == prev.height) {
                // _rect was moved out of boundary
                ret.y = height - ret.height;
            }
            else {
                // _rect was resized out of boundary
                if (ret.y <= height) {
                    ret.height = height - ret.y;
                }
                else {
                    ret.y = height;
                    ret.height = height;
                }
            }
        }

        return ret;
    }

    bool operator==(const _rect& o) const
    {
        return x == o.x && y == o.y && width == o.width && height == o.height;
    }

    bool operator!=(const _rect& o) const
    {
        return x != o.x || y != o.y || width != o.width || height != o.height;
    }
};

inline usize usize::expand(const urect& r) const
{
    return usize(std::max(width, r.right()), std::max(height, r.bottom()));
}

inline bool usize::contains(const urect& r) const
{
    return r.right() <= width && r.bottom() <= height;
}
}
