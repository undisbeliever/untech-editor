/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

namespace UnTech {
namespace MetaTiles {

constexpr unsigned MAX_GRID_WIDTH = 255;
constexpr unsigned MAX_GRID_HEIGHT = 255;

struct EngineSettings {
    unsigned maxMapSize;
    unsigned nMetaTiles;

    bool operator==(const EngineSettings& o) const
    {
        return maxMapSize == o.maxMapSize
               && nMetaTiles == o.nMetaTiles;
    }
    bool operator!=(const EngineSettings& o) const { return !(*this == o); }
};

constexpr static unsigned METATILE_SIZE_PX = 16;
}
}
