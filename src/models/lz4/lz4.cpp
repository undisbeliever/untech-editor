/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#if defined(__clang__)
#define LZ4_DISABLE_DEPRECATE_WARNINGS
#endif

#include "lz4.h"
#include "models/common/exceptions.h"
#include "models/common/stringbuilder.h"
#include "vendor/lz4/lib/lz4hc.h"
#include <span>

namespace UnTech {

std::vector<uint8_t>
lz4HcCompress(const std::vector<uint8_t>& source, unsigned limit)
{
    if (limit > UINT16_MAX
        || source.size() > UINT16_MAX) {

        throw runtime_error(u8"Cannot compress >= 64KiB of data");
    }

    if (source.empty()) {
        throw runtime_error(u8"Cannot compress an empty data block");
    }

    const int HEADER_SIZE = 2;
    int bound = LZ4_compressBound(source.size());

    std::vector<uint8_t> out(bound + HEADER_SIZE);
    out[0] = source.size() & 0xff;
    out[1] = (source.size() >> 8) & 0xff;

    const auto dest = std::span(out).subspan(HEADER_SIZE);

    const int cSize = LZ4_compress_HC(
        reinterpret_cast<const char*>(source.data()), // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<char*>(dest.data()),         // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        source.size(),
        dest.size(),
        LZ4HC_CLEVEL_MAX);

    if (cSize <= 0) {
        throw runtime_error(u8"LZ4_compress_HC failed");
    }

    unsigned outSize = cSize + HEADER_SIZE;
    if (outSize > limit) {
        throw runtime_error(u8"Compressed data exceeds limit (", outSize, u8" bytes, limit: ", limit, u8")");
    }

    out.resize(outSize);

    return out;
}

}
