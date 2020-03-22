/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "scenes.h"

namespace UnTech {
namespace Resources {

inline unsigned bitDepthForLayer(BgMode mode, unsigned layer)
{
    // BG4 always exists as it contains the status bar
    constexpr static std::array<unsigned, N_BG_MODES* N_LAYERS> bitDepths = {
        2, 2, 2, 2, // MODE_0
        4, 4, 2, 2, // MODE_1
        4, 4, 2, 2, // MODE_1_BG_PRIORITY
        4, 4, 0, 2, // MODE_2
        8, 4, 0, 2, // MODE_3
        8, 2, 0, 2, // MODE_4
    };

    return bitDepths.at(unsigned(mode) * N_LAYERS + (layer % N_LAYERS));
}
}
}
