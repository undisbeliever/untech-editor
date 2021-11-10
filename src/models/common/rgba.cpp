/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "rgba.h"
#include "stringbuilder.h"

namespace UnTech {

static_assert(sizeof(rgba) == 4, u8"rgba is the wrong size");

std::u8string rgba::rgbHexString() const
{
    return stringBuilder(hex_6(rgbHex()));
}

}
