#ifndef _UNTECH_MODELS_SNES_PALETTE_H
#define _UNTECH_MODELS_SNES_PALETTE_H

#include "snescolor.h"
#include "../common/ms8aabb.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace UnTech {
namespace Snes {

class Palette {
public:
    const static size_t N_COLORS = 16;

    typedef std::array<SnesColor, N_COLORS> palette_t;

public:
    Palette() = default;
    Palette(const Palette& p) = default;

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
}
}

#endif
