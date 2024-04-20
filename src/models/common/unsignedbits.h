/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <cstdint>
#include <type_traits>

namespace UnTech {

template <unsigned BITS, typename T = uint_fast16_t>
struct UnsignedBits {
    static_assert(BITS <= sizeof(T) * 8, "Too many bits");
    static_assert(std::is_unsigned<T>::value, "T must be unsigned");

private:
    T data;

public:
    constexpr static unsigned MASK = (1 << BITS) - 1;

    constexpr UnsignedBits()
        : data(0)
    {
    }

    constexpr UnsignedBits(const T v)
        : data(v & MASK)
    {
    }

    inline operator T() const { return data; }

    inline auto& operator=(const T v)
    {
        data = v & MASK;
        return *this;
    }
};
}
