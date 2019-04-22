/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

namespace UnTech {
namespace MetaTiles {

constexpr unsigned TILESET_WIDTH = 16;
constexpr unsigned TILESET_HEIGHT = 16;
constexpr unsigned N_METATILES = 256;

constexpr unsigned MAX_GRID_WIDTH = 255;
constexpr unsigned MAX_GRID_HEIGHT = 255;

struct EngineSettings {
    unsigned maxMapSize = 8192;

    bool operator==(const EngineSettings& o) const
    {
        return maxMapSize == o.maxMapSize;
    }
    bool operator!=(const EngineSettings& o) const { return !(*this == o); }
};

constexpr static unsigned METATILE_SIZE_PX = 16;
}
}
