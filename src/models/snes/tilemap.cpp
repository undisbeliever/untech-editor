/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilemap.h"
#include "models/common/stringbuilder.h"
#include <stdexcept>

using namespace UnTech::Snes;

Tilemap::Tilemap(unsigned width, unsigned height)
    : _width(width)
    , _height(height)
    , _maps(width * height)
{
    if (_width == 0 || _height == 0) {
        throw std::invalid_argument("Tilemap width/height cannot be 0");
    }
}

std::vector<uint8_t> Tilemap::snesData() const
{
    std::vector<uint8_t> out(_maps.size() * MAP_SIZE * MAP_SIZE * 2);
    uint8_t* outData = out.data();

    for (const auto& map : _maps) {
        for (const auto& cell : map) {
            outData[0] = cell.data & 0xff;
            outData[1] = cell.data >> 8;

            outData += 2;
        }
    }

    return out;
}

void Tilemap::readSnesData(const std::vector<uint8_t>& in)
{
    size_t expectedSize = _maps.size() * MAP_SIZE * MAP_SIZE * 2;

    if (in.size() != expectedSize) {
        throw std::runtime_error(stringBuilder(
            "Tilemap data is the incorrect size, expected ", expectedSize, " bytes"));
    }

    const uint8_t* inData = in.data();

    for (auto& map : _maps) {
        for (auto& cell : map) {
            cell.data = inData[0] | (inData[1] << 8);

            inData += 2;
        }
    }
}
