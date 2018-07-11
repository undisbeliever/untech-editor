/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palette.h"
#include <stdexcept>

using namespace UnTech;
using namespace UnTech::Snes;

template <size_t BIT_DEPTH>
inline std::vector<uint8_t> Palette<BIT_DEPTH>::paletteData() const
{
    std::vector<uint8_t> data(N_COLORS * 2);
    auto* ptr = data.data();

    for (const auto& c : _colors) {
        *ptr++ = c.data() & 0xFF;
        *ptr++ = c.data() >> 8;
    }

    return data;
}

template <size_t BIT_DEPTH>
inline void Palette<BIT_DEPTH>::readPalette(const std::vector<uint8_t>& data)
{
    if (data.size() != N_COLORS * 2) {
        throw std::runtime_error("Palette data must contain 32 bytes");
    }

    for (unsigned i = 0; i < N_COLORS; i++) {
        _colors[i].setData((data[i * 2 + 1] << 8) | data[i * 2]);
    }
}

template class Snes::Palette<1>;
template class Snes::Palette<2>;
template class Snes::Palette<3>;
template class Snes::Palette<4>;
template class Snes::Palette<8>;
