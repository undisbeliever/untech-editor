#ifndef _UNTECH_MODELS_COMMON_INT_MS8_T_H_
#define _UNTECH_MODELS_COMMON_INT_MS8_T_H_

#include <cstdint>

namespace UnTech {

/**
 * The number format used by UnTech's Metasprite system to handle signed
 * integers.
 *
 *
 * The data stored in ROM is in the following format:
 *
 *      byte value: (value - OFFSET)
 *
 * This is much faster than sign extending a byte value in 65816 assembly
 */

class int_ms8_t {
public:
    static const unsigned OFFSET = 128;

    static const int MIN = -OFFSET;
    static const int MAX = UINT8_MAX - OFFSET;

    static bool isValid(const int v) { return v >= MIN && v <= MAX; }

private:
    int_fast8_t data;

    inline void assign(const int v)
    {
        data = v >= MIN ? (v <= MAX ? v : MAX) : MIN;
    }

public:
    inline int_ms8_t()
        : data(0)
    {
    }
    inline int_ms8_t(const int_ms8_t& v)
        : data(v.data)
    {
    }
    inline int_ms8_t(const int& v) { assign(v); }

    inline operator int() const { return data; }
    inline auto& operator=(const int v)
    {
        assign(v);
        return *this;
    }

    inline auto& operator+=(const int v)
    {
        assign(data + v);
        return *this;
    }
    inline auto& operator-=(const int v)
    {
        assign(data - v);
        return *this;
    }
    inline auto& operator*=(const int v)
    {
        assign(data * v);
        return *this;
    }
    inline auto& operator/=(const int v)
    {
        assign(data / v);
        return *this;
    }
    inline auto& operator%=(const int v)
    {
        assign(data % v);
        return *this;
    }

    inline uint8_t romData() { return data + OFFSET; }
};
}
#endif
