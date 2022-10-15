/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/aabb.h"
#include "models/common/ms8aabb.h"

namespace UnTech::Gui {

struct TwoPointRect {
    // NOTE: This code assumes x2 > x1 and y2 > y1.

    int x1 = 0;
    int x2 = 1;
    int y1 = 0;
    int y2 = 1;

    TwoPointRect() = default;

    constexpr TwoPointRect(int x1_, int x2_, int y1_, int y2_)
        : x1(x1_)
        , x2(x2_)
        , y1(y1_)
        , y2(y2_)
    {
    }

    explicit TwoPointRect(const ms8rect& r)
        : TwoPointRect(r.x, r.x + r.width, r.y, r.y + r.height)
    {
    }

    explicit TwoPointRect(const urect& r)
        : TwoPointRect(r.x, r.x + r.width, r.y, r.y + r.height)
    {
    }

    explicit TwoPointRect(const rect& r)
        : TwoPointRect(r.x, r.x + r.width, r.y, r.y + r.height)
    {
    }

    [[nodiscard]] int width() const { return x2 - x1; }
    [[nodiscard]] int height() const { return y2 - y1; }

    [[nodiscard]] bool contains(int x, int y) const
    {
        return x >= this->x1 && x < this->x2
               && y >= this->y1 && y <= this->y2;
    }

    [[nodiscard]] bool contains(const point p) const
    {
        return p.x >= this->x1 && p.x < this->x2
               && p.y >= this->y1 && p.y <= this->y2;
    }

    [[nodiscard]] bool contains(const TwoPointRect r) const
    {
        return r.x1 >= this->x1 && r.x2 <= this->x2
               && r.y1 >= this->y1 && r.y2 <= this->y2;
    }
};

}
