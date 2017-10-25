/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

namespace UnTech {
namespace MetaTiles {

struct EngineSettings {
    unsigned maxMapSize;
    unsigned nMetaTiles;
};

constexpr static unsigned METATILE_SIZE_PX = 16;
}
}
