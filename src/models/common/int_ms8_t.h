/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "clampedinteger.h"
#include <cstdint>

namespace UnTech {

/**
 * The number format used by UnTech's Metasprite system to handle signed
 * integers.
 *
 *
 * The data stored in ROM is in the following format:
 *
 *      ROM value: (integer value + OFFSET)
 *
 * This is much faster than sign extending a byte value in 65816 assembly
 */

class int_ms8_t : public ClampedType<int16_t, -128, 127> {
public:
    static const unsigned OFFSET = 128;
    static bool isValid(const int v) { return v >= MIN && v <= MAX; }

    int_ms8_t() = default;
    int_ms8_t(const int_fast16_t& v)
        : ClampedType(v)
    {
    }

    inline uint8_t romData() const { return *this + OFFSET; }
};
}
