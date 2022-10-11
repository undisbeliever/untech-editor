/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "aabb.h"
#include "vectorset.h"
#include <tuple>

namespace UnTech {

struct upoint_compare {
    bool operator()(const upoint& a, const upoint& b)
    {
        return std::tie(a.x, a.y) < std::tie(b.x, b.y);
    }
};

using upoint_vectorset = vectorset<upoint, upoint_compare>;
}
