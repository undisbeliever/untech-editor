/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "snescolor.h"
#include "../common/ms8aabb.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace UnTech {
namespace Snes {

template <size_t BIT_DEPTH>
class Palette {
public:
    static_assert(BIT_DEPTH <= 8, "BIT_DEPTH too large");

    constexpr static size_t N_COLORS = 1 << BIT_DEPTH;
    constexpr static unsigned PIXEL_MASK = (1 << BIT_DEPTH) - 1;

    using palette_t = std::array<SnesColor, N_COLORS>;

public:
    Palette() = default;
    Palette(const std::array<uint8_t, N_COLORS * 2>& data)
        : _colors()
    {
        readPaletteData(data);
    }

    inline palette_t& colors() { return _colors; }
    inline const palette_t& colors() const { return _colors; }

    inline SnesColor& color(size_t pos) { return _colors[pos]; }
    inline const SnesColor& color(size_t pos) const { return _colors[pos]; }

    std::array<uint8_t, N_COLORS * 2> paletteData() const;

    /**
     * Reads the palette data into this palette instance.
     */
    void readPaletteData(const std::array<uint8_t, N_COLORS * 2>& data);

    bool operator==(const Palette& o) const { return _colors == o._colors; }
    bool operator!=(const Palette& o) const { return _colors != o._colors; }

private:
    palette_t _colors;
};

typedef Palette<1> Palette1bpp;
typedef Palette<2> Palette2bpp;
typedef Palette<3> Palette3bpp;
typedef Palette<4> Palette4bpp;
typedef Palette<8> Palette8bpp;
}
}
