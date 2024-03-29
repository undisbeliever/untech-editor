/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilemap.h"
#include "models/common/exceptions.h"
#include "models/common/stringbuilder.h"

namespace UnTech::Snes {

Tilemap::Tilemap(unsigned width, unsigned height)
    : _width(width)
    , _height(height)
    , _maps(width * height)
{
    if (_width == 0 || _height == 0) {
        throw invalid_argument(u8"Tilemap width/height cannot be 0");
    }
}

std::vector<uint8_t> Tilemap::snesData() const
{
    std::vector<uint8_t> out(_maps.size() * MAP_SIZE * MAP_SIZE * 2);
    auto outIt = out.begin();

    for (const auto& m : _maps) {
        for (const auto& cell : m) {
            *outIt++ = cell.data & 0xff;
            *outIt++ = cell.data >> 8;
        }
    }
    assert(outIt == out.end());

    return out;
}

}
