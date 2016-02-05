#ifndef _UNTECH_MODELS_COMMON_AABB_H_
#define _UNTECH_MODELS_COMMON_AABB_H_

#include <algorithm>

namespace UnTech {

struct urect;

struct upoint {
    unsigned x;
    unsigned y;

    bool operator==(const upoint& o)
    {
        return x == o.x && y == o.y;
    }

    bool operator!=(const upoint& o)
    {
        return x != o.x || y != o.y;
    }
};

struct usize {
    unsigned width;
    unsigned height;

    inline usize expand(const usize& s)
    {
        return { std::max(width, s.width), std::max(height, s.height) };
    }

    inline usize expand(const upoint& p)
    {
        return { std::max(width, p.x), std::max(height, p.y) };
    }

    inline usize expand(const urect& r);

    inline upoint clip(const upoint& p)
    {
        return { std::min(width, p.x), std::min(height, p.y) };
    }

    bool operator==(const usize& o)
    {
        return width == o.width && height == o.height;
    }

    bool operator!=(const usize& o)
    {
        return width != o.width || height != o.height;
    }
};

struct urect {
    unsigned x;
    unsigned y;
    unsigned width;
    unsigned height;

    inline unsigned left() const { return x; }
    inline unsigned right() const { return x + width; }
    inline unsigned top() const { return y; }
    inline unsigned bottom() const { return y + height; }

    inline usize size() const { return { width, height }; }

    // Clips the upoint within the width/height of the urect
    inline upoint clipInside(const upoint& p)
    {
        return { std::min(p.x, width), std::min(p.y, height) };
    }

    // Clips the upoint/usize within the width/height of the urect
    upoint clipInside(const upoint& p, unsigned squareSize)
    {
        upoint ret = { 0, 0 };

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
    urect clipInside(const urect& r, const urect& prev)
    {
        urect ret = r;

        if (ret.right() > width) {
            if (ret.width == prev.width) {
                // urect was moved out of boundary
                ret.x = width - ret.width;
            }
            else {
                // urect was resized out of boundary
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
                // urect was moved out of boundary
                ret.y = height - ret.height;
            }
            else {
                // urect was resized out of boundary
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

    bool operator==(const urect& o)
    {
        return x == o.x && y == o.y && width == o.width && height == o.height;
    }

    bool operator!=(const urect& o)
    {
        return x != o.x || y != o.y || width != o.width || height != o.height;
    }
};

inline usize usize::expand(const urect& r)
{
    return { std::max(width, r.right()), std::max(height, r.bottom()) };
}
}
#endif
