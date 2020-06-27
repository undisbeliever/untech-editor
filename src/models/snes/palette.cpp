/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palette.h"
#include <cassert>
#include <stdexcept>

using namespace UnTech;
using namespace UnTech::Snes;

template <size_t BIT_DEPTH>
auto Palette<BIT_DEPTH>::paletteData() const -> std::array<uint8_t, N_COLORS * 2>
{
    std::array<uint8_t, std::tuple_size<palette_t>::value * 2> data;
    auto it = data.begin();

    for (const auto& c : _colors) {
        *it++ = c.data() & 0xFF;
        *it++ = c.data() >> 8;
    }
    assert(it == data.end());

    return data;
}

// ::TODO replace with std::span when upgrading to c++20::
template <size_t BIT_DEPTH>
void Palette<BIT_DEPTH>::readPaletteData(const std::array<uint8_t, N_COLORS * 2>& data)
{
    for (unsigned i = 0; i < N_COLORS; i++) {
        _colors[i].setData((data[i * 2 + 1] << 8) | data[i * 2]);
    }
}

template class Snes::Palette<1>;
template class Snes::Palette<2>;
template class Snes::Palette<3>;
template class Snes::Palette<4>;
template class Snes::Palette<8>;
