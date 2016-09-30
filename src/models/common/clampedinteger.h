#pragma once

#include <climits>
#include <cstdint>
#include <type_traits>

namespace UnTech {

template <typename T, T MIN_VALUE, T MAX_VALUE>
struct ClampedType {
    static_assert(std::is_integral<T>::value, "Integer required");

    static const T MIN = MIN_VALUE;
    static const T MAX = MAX_VALUE;
    using TYPE = T;
    using COMP_TYPE = typename std::conditional<std::is_signed<T>::value, int_fast32_t, uint_fast32_t>::type;

    // COMP_TYPE exists to prevent Narrowing conversion and ensure
    // that ClampedType<uint8, 1, 20>(1258) works correctly
    static_assert(sizeof(COMP_TYPE) > sizeof(TYPE), "T too large");

private:
    T data;

public:
    ~ClampedType() = default;
    ClampedType(const ClampedType&) = default;
    ClampedType(ClampedType&&) = default;
    ClampedType& operator=(const ClampedType&) = default;
    ClampedType& operator=(ClampedType&&) = default;

    inline ClampedType() = default;

    inline ClampedType(const COMP_TYPE v)
    {
        data = v >= MIN ? (v <= MAX ? v : MAX) : MIN;
    }

    inline operator COMP_TYPE() const { return data; }

    inline auto& operator=(const COMP_TYPE v)
    {
        data = v >= MIN ? (v <= MAX ? v : MAX) : MIN;
    }
};

template <unsigned MIN_VALUE, unsigned MAX_VALUE>
using ClampedUnsigned = ClampedType<unsigned, MIN_VALUE, MAX_VALUE>;

template <int MIN_VALUE, int MAX_VALUE>
using ClampedInteger = ClampedType<int, MIN_VALUE, MAX_VALUE>;
}
