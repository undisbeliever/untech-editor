/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "snescolor.h"

using namespace UnTech::Snes;

void SnesColor::setRgb(const rgba color)
{
    // Ignoring rounding so converting SNES->RGB->SNES is lossless

    unsigned b = (color.blue >> 3) & 31;
    unsigned g = (color.green >> 3) & 31;
    unsigned r = (color.red >> 3) & 31;

    _data = (b << 10) | (g << 5) | r;

    updateRgb();
}

void SnesColor::setData(const uint16_t data)
{
    _data = data & 0x7FFF;
    updateRgb();
}

void SnesColor::setBlue(const uint8_t blue)
{
    uint8_t b = blue & 31;

    _data = (_data & (0x7FFF ^ (31 << 10))) | (b << 10);
    updateRgb();
}

void SnesColor::setGreen(const uint8_t green)
{
    uint8_t g = green & 31;

    _data = (_data & (0x7FFF ^ (31 << 5))) | (g << 5);
    updateRgb();
}

void SnesColor::setRed(const uint8_t red)
{
    uint8_t r = red & 31;

    _data = (_data & (0x7FFF ^ 31)) | r;
    updateRgb();
}

void SnesColor::updateRgb()
{
    // ::KUDOS ccovell http://forums.nesdev.com/viewtopic.php?p=146491#p146491::

    unsigned b = blue();
    unsigned g = green();
    unsigned r = red();

    _rgb.blue = (b << 3) | (b >> 2);
    _rgb.green = (g << 3) | (g >> 2);
    _rgb.red = (r << 3) | (r >> 2);
    _rgb.alpha = 0xFF;
}
