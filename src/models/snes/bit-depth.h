/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

namespace UnTech::Snes {

enum class BitDepth : unsigned {
    BD_2BPP = 2,
    BD_4BPP = 4,
    BD_8BPP = 8,
};
BitDepth toBitDepth(unsigned bd);

enum class BitDepthSpecial : unsigned {
    BD_1BPP = 1,
    BD_2BPP = 2,
    BD_3BPP = 3,
    BD_4BPP = 4,
    BD_8BPP = 8,
};
BitDepthSpecial toBitDepthSpecial(unsigned bd);

inline constexpr unsigned colorsForBitDepth(BitDepth bd)
{
    return 1U << static_cast<unsigned>(bd);
}

inline constexpr unsigned colorsForBitDepth(BitDepthSpecial bd)
{
    return 1U << static_cast<unsigned>(bd);
}

inline constexpr unsigned pixelMaskForBitdepth(BitDepth bd)
{
    return (1U << static_cast<unsigned>(bd)) - 1;
}

inline constexpr unsigned pixelMaskForBitdepth(BitDepthSpecial bd)
{
    return (1U << static_cast<unsigned>(bd)) - 1;
}

// Size of an 8x8 tile on the SNES for the given bitdepth
inline constexpr unsigned snesTileSizeForBitdepth(BitDepth bd)
{
    return 8 * static_cast<unsigned>(bd);
}

inline constexpr unsigned snesTileSizeForBitdepth(BitDepthSpecial bd)
{
    return 8 * static_cast<unsigned>(bd);
}

}
