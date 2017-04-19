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

    const static size_t N_COLORS = 1 << BIT_DEPTH;
    const static unsigned PIXEL_MASK = (1 << BIT_DEPTH) - 1;

    typedef std::array<SnesColor, N_COLORS> palette_t;

public:
    Palette() = default;
    Palette(const std::vector<uint8_t>& data)
        : _colors()
    {
        readPalette(data);
    }

    inline palette_t& colors() { return _colors; }
    inline const palette_t& colors() const { return _colors; }

    inline SnesColor& color(size_t pos) { return _colors[pos]; }
    inline const SnesColor& color(size_t pos) const { return _colors[pos]; }

    /**
     * Returns a vector containing 32 bytes which houses the SNES
     * data format of the palette.
     */
    std::vector<uint8_t> paletteData() const;

    /**
     * Reads the palette data into this palette instance.
     *
     * If vector does not contain 32 bytes an exception will be thrown.
     */
    void readPalette(const std::vector<uint8_t>& data);

private:
    palette_t _colors;
};

typedef Palette<2> Palette2bpp;
typedef Palette<4> Palette4bpp;
typedef Palette<8> Palette8bpp;
}
}
