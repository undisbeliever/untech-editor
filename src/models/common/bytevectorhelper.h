/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <cassert>
#include <vector>

namespace UnTech {

inline static void writeUint8(std::vector<uint8_t>& out, unsigned value)
{
    assert(value <= 0xff);
    out.push_back(value);
}

inline static void writeUint16(std::vector<uint8_t>& out, unsigned value)
{
    assert(value <= 0xffff);
    out.push_back(value & 0xff);
    out.push_back((value >> 8) & 0xff);
}
}
