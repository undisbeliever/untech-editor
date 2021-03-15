/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "snescolor.h"

using namespace UnTech::Snes;

void SnesColor::setData(const uint16_t data)
{
    _data = data & 0x7FFF;
}

void SnesColor::setBlue(const uint8_t blue)
{
    uint8_t b = blue & 31;

    _data = (_data & (0x7FFF ^ (31 << 10))) | (b << 10);
}

void SnesColor::setGreen(const uint8_t green)
{
    uint8_t g = green & 31;

    _data = (_data & (0x7FFF ^ (31 << 5))) | (g << 5);
}

void SnesColor::setRed(const uint8_t red)
{
    uint8_t r = red & 31;

    _data = (_data & (0x7FFF ^ 31)) | r;
}
