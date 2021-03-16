/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

namespace UnTech {

// Does not crash with an assert failure if `min > max`.
template <typename T>
inline T clamp(const T v, const T min, const T max)
{
    if (v < min) {
        return min;
    }
    else if (v >= max) {
        return max;
    }
    else {
        return v;
    }
}

}
