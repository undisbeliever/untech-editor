#pragma once

namespace UnTech {

template <typename T, T MIN_VALUE, T MAX_VALUE>
struct ClampedInteger {
    static const T MIN = MIN_VALUE;
    static const T MAX = MAX_VALUE;

private:
    T data;

    inline auto& _op(const T v)
    {
        data = v > MIN ? (v < MAX ? v : MAX) : MIN;
        return *this;
    }

public:
    inline ClampedInteger() = default;
    inline ClampedInteger(const ClampedInteger& o) = default;
    inline ClampedInteger(const T& v) { _op(v); }

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
