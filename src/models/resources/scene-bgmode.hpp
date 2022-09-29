/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "scenes.h"
#include "models/snes/bit-depth.h"

namespace UnTech::Resources {

inline unsigned numberOfLayers(const BgMode mode)
{
    switch (mode) {
    case BgMode::MODE_0:
        return 4;

    case BgMode::MODE_1:
    case BgMode::MODE_1_BG3_PRIOTITY:
        return 3;

    case BgMode::MODE_2:
    case BgMode::MODE_3:
    case BgMode::MODE_4:
        return 2;
    }

    return 0;
}

inline std::optional<Snes::BitDepth> bitDepthForLayer(BgMode mode, unsigned layer)
{
    using B = Snes::BitDepth;

    constexpr static std::array<std::optional<Snes::BitDepth>, (N_BG_MODES * N_LAYERS)> bitDepths = {
        B::BD_2BPP, B::BD_2BPP, B::BD_2BPP, B::BD_2BPP,     // MODE_0
        B::BD_4BPP, B::BD_4BPP, B::BD_2BPP, std::nullopt,   // MODE_1
        B::BD_4BPP, B::BD_4BPP, B::BD_2BPP, std::nullopt,   // MODE_1_BG_PRIORITY
        B::BD_4BPP, B::BD_4BPP, std::nullopt, std::nullopt, // MODE_2
        B::BD_8BPP, B::BD_4BPP, std::nullopt, std::nullopt, // MODE_3
        B::BD_8BPP, B::BD_2BPP, std::nullopt, std::nullopt, // MODE_4
    };

    return bitDepths.at(unsigned(mode) * N_LAYERS + (layer % N_LAYERS));
}

}
