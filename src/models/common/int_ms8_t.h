/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

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
class int_ms8_t final {
public:
    static constexpr int MIN = -128;
    static constexpr int MAX = 127;

    static constexpr unsigned OFFSET = 128;

    static_assert(MAX - MIN == UINT8_MAX);
    static_assert(MAX + OFFSET == UINT8_MAX);

    static constexpr bool isValid(const int v) { return v >= MIN && v <= MAX; }

    static constexpr int_fast16_t clamp(int v) { return v >= MIN ? (v <= MAX ? v : MAX) : MIN; }

private:
    int_fast16_t value;

public:
    // cppcheck-suppress noExplicitConstructor
    constexpr int_ms8_t(const int v = 0)
        : value(clamp(v))
    {
    }

    inline operator int_fast16_t() const { return value; }

    inline auto& operator=(const int v)
    {
        value = clamp(v);
        return *this;
    }

    inline uint8_t romData() const { return value + OFFSET; }
};
}
