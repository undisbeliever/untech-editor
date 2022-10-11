/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "snescolor.h"
#include "models/common/rgba.h"

namespace UnTech::Snes {

inline SnesColor toSnesColor(const rgba& color)
{
    // Ignoring rounding so converting SNES->RGB->SNES is lossless
    unsigned b = (color.blue >> 3) & 31;
    unsigned g = (color.green >> 3) & 31;
    unsigned r = (color.red >> 3) & 31;

    return SnesColor((b << 10) | (g << 5) | r);
}

inline rgba toRgb(const SnesColor& c)
{
    // ::KUDOS ccovell http://forums.nesdev.com/viewtopic.php?p=146491#p146491::

    unsigned b = c.blue();
    unsigned g = c.green();
    unsigned r = c.red();

    return {
        uint8_t((r << 3) | (r >> 2)),
        uint8_t((g << 3) | (g >> 2)),
        uint8_t((b << 3) | (b >> 2))
    };
}

}
