#pragma once

#include "palette.h"
#include <stdexcept>

namespace UnTech {
namespace Snes {

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
}
}
