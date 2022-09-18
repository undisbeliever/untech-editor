/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "bit.h"
#include "stringbuilder.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace UnTech {

class StringStream {

public:
    constexpr static size_t default_initial_size = 16 * 1024;

    // Limit maximum size of a StringStream to 1 MiB
    constexpr static size_t max_buffer_size = 1024 * 1024;

private:
    std::u8string buffer;
    size_t bufferPos;

private:
    // Disabling copying/moving
    StringStream(const StringStream&) = delete;
    StringStream(StringStream&&) = delete;
    StringStream& operator=(const StringStream&) = delete;
    StringStream& operator=(StringStream&&) = delete;

private:
    // NOTE: Will always expand buffer
    void expandBuffer(const size_t capacity);

    template <typename... Args>
    void writeImpl(const Args&... args)
    {
        const size_t toAddEstimate = (... + UnTech::StringBuilder::stringSize(args)) + 1;
        const size_t bufferPosEstimate = bufferPos + toAddEstimate;

        if (bufferPosEstimate >= buffer.size()) {
            expandBuffer(bufferPosEstimate);
        }

        // Using `char*` here as `std::to_chars()` uses pointers.
        char8_t* ptr = buffer.data() + bufferPos;
        char8_t* const end = buffer.data() + buffer.size();

        auto process = [&](const auto& a) {
            ptr = UnTech::StringBuilder::concat(ptr, end, a);
        };
        (process(args), ...);

        assert(ptr >= buffer.data() && ptr < end);

        bufferPos = std::distance(buffer.data(), ptr);

        assert(bufferPos < bufferPosEstimate);
    }

    static size_t initialBufferSize(size_t startingSize)
    {
        if (startingSize < 1024) {
            throw std::invalid_argument("Invalid StringStream startingSize");
        }

        if (not isPowerOfTwo(startingSize)) {
            throw std::invalid_argument("StringStream startingSize is not a power of two");
        }

        // -1 to account for the null terminating character when allocating memory in std::u8string
        return startingSize - 1;
    }

public:
    explicit StringStream(size_t startingSize = default_initial_size)
        : buffer(initialBufferSize(startingSize), '\0')
        , bufferPos(0)
    {
    }

    // Resizes the buffer if it cannot hold `toAddExtimage`
    inline void addToBufferTest(const size_t toAddEstimate)
    {
        const size_t requiredCapacity = bufferPos + toAddEstimate;

        if (requiredCapacity >= buffer.size()) {
            expandBuffer(requiredCapacity);
        }
    }

    template <size_t N>
    void writeCharacters(const std::array<char8_t, N>& array)
    {
        addToBufferTest(array.size());

        char8_t* ptr = buffer.data() + bufferPos;

        std::copy(array.begin(), array.end(), ptr);

        bufferPos += array.size();
    }

    template <typename... Args>
    inline void write(const Args&... args)
    {
        writeImpl(UnTech::StringBuilder::convert(args)...);
    }

    void write(const std::u8string_view s);

    void write(const std::u8string& s);

    inline void write(const char8_t* s) { write(std::u8string_view(s)); }

    inline size_t size() const { return bufferPos; }

    // returned string_view is only valid until the next write call.
    [[nodiscard]] std::u8string_view string_view() const
    {
        return std::u8string_view(buffer.data(), bufferPos);
    }

    // Takes the string out of the StringBuilder
    [[nodiscard]] std::u8string takeString()
    {
        assert(bufferPos < buffer.size());

        buffer.erase(buffer.begin() + bufferPos, buffer.end());

        // Replace `buffer` with an empty string
        std::u8string s;
        std::swap(s, buffer);

        bufferPos = 0;

        return s;
    }
};

}
