#include "palette.h"
#include "models/common/humantypename.h"
#include <stdexcept>

using namespace UnTech;
using namespace UnTech::Snes;

template <>
const std::string HumanTypeName<Palette<2>>::value = "Palette";

template <>
const std::string HumanTypeName<Palette<4>>::value = "Palette";

template <>
const std::string HumanTypeName<Palette<8>>::value = "Palette";

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

template class Snes::Palette<2>;
template class Snes::Palette<4>;
template class Snes::Palette<8>;
