/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <cstdint>
#include <vector>

namespace std {

template <>
struct hash<std::vector<uint8_t>> {
    inline size_t operator()(const std::vector<uint8_t>& vec) const
    {
        size_t seed = vec.size();

        for (uint8_t v : vec) {
            // numbers from boost
            seed ^= v + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        return seed;
    }
};
}
