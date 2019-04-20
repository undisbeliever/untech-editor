/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/rgba.h"
#include <cstdint>

namespace UnTech {
namespace Snes {

class SnesColor {
public:
    ~SnesColor() = default;
    SnesColor(const SnesColor&) = default;
    SnesColor(SnesColor&&) = default;
    SnesColor& operator=(const SnesColor&) = default;
    SnesColor& operator=(SnesColor&&) = default;

    SnesColor()
        : _rgb(0, 0, 0, 0xFF)
        , _data(0)
    {
    }

    SnesColor(const rgba& color)
        : _rgb(0, 0, 0, 0xFF)
        , _data(0)
    {
        setRgb(color);
    }

    SnesColor(const uint16_t value)
        : _rgb(0, 0, 0, 0xFF)
        , _data(0)
    {
        setData(value);
    }

    // will never match a valid SnesColor
    static SnesColor invalidColor()
    {
        SnesColor c;
        c._rgb.alpha = 0;
        c._data = 0xffff;
        return c;
    }

    inline const rgba& rgb() const { return _rgb; }
    inline const uint16_t& data() const { return _data; }

    inline uint8_t blue() const { return (_data >> 10) & 31; }
    inline uint8_t green() const { return (_data >> 5) & 31; }
    inline uint8_t red() const { return _data & 31; }

    void setRgb(const rgba color);
    void setData(const uint16_t data);

    void setBlue(const uint8_t blue);
    void setGreen(const uint8_t green);
    void setRed(const uint8_t red);

    inline bool operator==(const SnesColor& o) const { return _data == o._data; }
    inline bool operator!=(const SnesColor& o) const { return _data != o._data; }

private:
    void updateRgb();

private:
    rgba _rgb;
    uint16_t _data;
};
}
}
