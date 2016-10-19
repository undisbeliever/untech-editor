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

    inline auto& _op(const T v)
    {
        data = v & MASK;
        return *this;
    }

public:
    const static unsigned MASK = (1 << BITS) - 1;

    ~UnsignedBits() = default;
    UnsignedBits(const UnsignedBits&) = default;
    UnsignedBits(UnsignedBits&&) = default;
    UnsignedBits& operator=(const UnsignedBits&) = default;
    UnsignedBits& operator=(UnsignedBits&&) = default;

    inline UnsignedBits()
        : data(0)
    {
    }

    inline UnsignedBits(const T& v)
        : data(v & MASK)
    {
    }

    inline operator T() const { return data; }

    inline auto& operator=(const T v) { return _op(v); }
    inline auto& operator&=(const T v) { return _op(data & v); }
    inline auto& operator|=(const T v) { return _op(data | v); }
    inline auto& operator^=(const T v) { return _op(data ^ v); }
    inline auto& operator+=(const T v) { return _op(data + v); }
    inline auto& operator-=(const T v) { return _op(data - v); }
    inline auto& operator*=(const T v) { return _op(data * v); }
    inline auto& operator/=(const T v) { return _op(data / v); }
    inline auto& operator%=(const T v) { return _op(data % v); }
    inline auto& operator<<=(const T v) { return _op(data << v); }
    inline auto& operator>>=(const T v) { return _op(data >> v); }
};
}
