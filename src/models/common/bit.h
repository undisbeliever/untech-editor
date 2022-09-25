/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <stdint.h>

namespace UnTech {

inline constexpr bool isPowerOfTwo(int64_t v)
{
    return v && (v & (v - 1)) == 0;
}

inline constexpr unsigned nextPowerOfTwo(unsigned i)
{
    unsigned p = 1;
    while (p < i) {
        p <<= 1;
    }
    return p;
}

}
