/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "stringstream.h"

namespace UnTech {

void StringStream::expandBuffer(const size_t capacity)
{
    // ASSUME `startingSize` is a power of two

    size_t newBufferSize = (buffer.size() + 1) * 2;
    if (newBufferSize == 0) {
        newBufferSize = default_initial_size;
    }

    while (newBufferSize < capacity) {
        newBufferSize *= 2;
    }

    assert(isPowerOfTwo(newBufferSize));

    if (newBufferSize > max_buffer_size) {
        throw std::length_error("StringStream: newBufferSize > max_buffer_size");
    }

    // -1 to account for the null terminating character when allocating memory in std::string
    buffer.resize(newBufferSize - 1);
}

void StringStream::write(const std::string_view s)
{
    addToBufferTest(s.size());

    char* ptr = buffer.data() + bufferPos;

    std::copy(s.begin(), s.end(), ptr);

    bufferPos += s.size();
}

void StringStream::write(const std::string& s)
{
    addToBufferTest(s.size());

    char* ptr = buffer.data() + bufferPos;

    std::copy(s.begin(), s.end(), ptr);

    bufferPos += s.size();
}

}
