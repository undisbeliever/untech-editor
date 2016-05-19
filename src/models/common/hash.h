#ifndef _UNTECH_MODELS_COMMON_HASH_H
#define _UNTECH_MODELS_COMMON_HASH_H

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

#endif
