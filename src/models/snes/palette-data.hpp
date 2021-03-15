/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "snescolor.h"
#include "models/common/iterators.h"
#include <array>
#include <cassert>

namespace UnTech::Snes {

template <size_t N_COLORS>
std::array<uint8_t, N_COLORS * 2> snesPaletteData(const std::array<SnesColor, N_COLORS>& palette)
{
    std::array<uint8_t, N_COLORS * 2> data;
    auto it = data.begin();

    for (const auto& c : palette) {
        *it++ = c.data() & 0xFF;
        *it++ = c.data() >> 8;
    }
    assert(it == data.end());

    return data;
}

template <size_t N_BYTES>
std::array<SnesColor, N_BYTES / 2> readSnesPaletteData(const std::array<uint8_t, N_BYTES>& data)
{
    static_assert(N_BYTES % 2 == 0);
    constexpr unsigned N_COLORS = N_BYTES / 2;

    std::array<SnesColor, N_COLORS> palette;

    for (const auto i : range(N_COLORS)) {
        palette[i].setData((data[i * 2 + 1] << 8) | data[i * 2]);
    }

    return palette;
}

}
