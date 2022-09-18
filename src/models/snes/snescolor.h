/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/rgba.h"
#include <array>
#include <cstdint>

namespace UnTech::Snes {

class SnesColor {
private:
    uint16_t _data;

public:
    ~SnesColor() = default;
    SnesColor(const SnesColor&) = default;
    SnesColor(SnesColor&&) = default;
    SnesColor& operator=(const SnesColor&) = default;
    SnesColor& operator=(SnesColor&&) = default;

    SnesColor()
        : _data(0)
    {
    }

    explicit SnesColor(const uint16_t value)
        : _data(value & 0x7FFF)
    {
    }

    inline const uint16_t& data() const { return _data; }

    inline uint8_t blue() const { return (_data >> 10) & 31; }
    inline uint8_t green() const { return (_data >> 5) & 31; }
    inline uint8_t red() const { return _data & 31; }

    void setData(const uint16_t data);

    void setBlue(const uint8_t blue);
    void setGreen(const uint8_t green);
    void setRed(const uint8_t red);

    bool operator==(const SnesColor&) const = default;
};

using Palette4bpp = std::array<SnesColor, 16>;

}
