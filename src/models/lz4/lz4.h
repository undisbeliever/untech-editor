/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <cstdint>
#include <vector>

namespace UnTech {

// Compress a data block with LZ4 HC at the best compression level.
//
// Throws an exception if:
//  * LZ4_compress_HC failed.
//  * the size of source cannot fit in a 16 bit unsigned integer.
//  * the compressed output is greater than `limit` bytes in size.
//
// NOTE: This function outputs a modified block frame.
//
// OUTPUT FORMAT:
//      <uint16 decompressed size> <lz4 compressed block>
std::vector<uint8_t> lz4HcCompress(const std::vector<uint8_t>& source, unsigned limit = UINT16_MAX);
}
