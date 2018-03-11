/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

namespace UnTech {

template <typename T>
void moveVectorItem(size_t from, size_t to, std::vector<T>& v)
{
    assert(from < v.size());
    assert(to < v.size());

    if (from == to) {
        return;
    }

    if (from < to) {
        std::rotate(v.begin() + from, v.begin() + from + 1, v.begin() + to + 1);
    }
    else {
        std::rotate(v.begin() + to, v.begin() + from, v.begin() + from + 1);
    }
}
}
