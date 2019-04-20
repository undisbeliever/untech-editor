/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

namespace UnTech {

template <class ListT>
void moveListItem(size_t from, size_t to, ListT& l)
{
    assert(from < l.size());
    assert(to < l.size());

    if (from == to) {
        return;
    }

    if (from < to) {
        std::rotate(l.begin() + from, l.begin() + from + 1, l.begin() + to + 1);
    }
    else {
        std::rotate(l.begin() + to, l.begin() + from, l.begin() + from + 1);
    }
}

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
