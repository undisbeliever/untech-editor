/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <string>
#include <vector>

namespace UnTech {

template <typename T>
inline void validateNotEmpty(const std::vector<T>& vec, const char* msg)
{
    if (vec.size() == 0) {
        throw std::runtime_error(msg);
    }
}

inline void validateMax(unsigned value, unsigned max, const char* msg)
{
    if (value > max) {
        throw std::runtime_error(msg
                                 + std::string("(") + std::to_string(value)
                                 + ", max: " + std::to_string(max) + ")");
    }
}
}
