/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

namespace UnTech::Snes {

inline constexpr unsigned colorsForBitDepth(unsigned bitDepth)
{
    return 1U << bitDepth;
}

inline constexpr unsigned pixelMaskForBitdepth(unsigned bitDepth)
{
    return (1U << bitDepth) - 1;
}

// Size of an 8x8 tile on the SNES for the given bitdepth
inline constexpr unsigned snesTileSizeForBitdepth(unsigned bitDepth)
{
    return 8 * bitDepth;
}

}
