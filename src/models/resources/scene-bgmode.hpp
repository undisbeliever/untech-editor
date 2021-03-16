/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "scenes.h"

namespace UnTech {
namespace Resources {

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

inline unsigned bitDepthForLayer(BgMode mode, unsigned layer)
{
    constexpr static std::array<unsigned, (N_BG_MODES * N_LAYERS)> bitDepths = {
        2, 2, 2, 2, // MODE_0
        4, 4, 2, 0, // MODE_1
        4, 4, 2, 0, // MODE_1_BG_PRIORITY
        4, 4, 0, 0, // MODE_2
        8, 4, 0, 0, // MODE_3
        8, 2, 0, 0, // MODE_4
    };

    return bitDepths.at(unsigned(mode) * N_LAYERS + (layer % N_LAYERS));
}
}
}
