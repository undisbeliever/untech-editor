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

private:
    std::u8string buffer;

private:
    // Suggested maximum size of the StringStream.
    //
    // NOTE: the buffer size is checked before the string is written.
    //       This limit is intended to catch infinite write loops, not large writes.
    constexpr static size_t max_buffer_size = 1024 * 1024;

    // MUST be called at the start of any `write()` method.
    void checkSize() const
    {
        if (buffer.size() > max_buffer_size) {
            throw std::length_error("StringStream: buffer size > max_buffer_size");
        }
    }

public:
    // Disabling copying/moving
    StringStream(const StringStream&) = delete;
    StringStream(StringStream&&) = delete;
    StringStream& operator=(const StringStream&) = delete;
    StringStream& operator=(StringStream&&) = delete;

public:
    explicit StringStream(size_t startingSize = default_initial_size)
        : buffer()
    {
        if (startingSize < 1024) {
            throw std::invalid_argument("Invalid StringStream startingSize");
        }

        if (startingSize > max_buffer_size) {
            throw std::invalid_argument("StringStream startingSize is too large");
        }

        if (not isPowerOfTwo(startingSize)) {
            throw std::invalid_argument("StringStream startingSize is not a power of two");
        }

        buffer.reserve(startingSize);
    }

    template <size_t N>
    void writeCharacters(const std::array<char8_t, N>& array)
    {
        checkSize();
        buffer.append(array.begin(), array.end());
    }

    template <typename... Args>
    inline void write(const Args&... args)
    {
        checkSize();
        (UnTech::StringBuilder::concat(buffer, args), ...);
    }

    void write(const std::u8string& s)
    {
        checkSize();
        buffer.append(s);
    }

    void write(const std::u8string_view s)
    {
        checkSize();
        buffer.append(s);
    }

    void write(const char8_t* s)
    {
        checkSize();
        buffer.append(std::u8string_view(s));
    }

    [[nodiscard]] inline size_t size() const { return buffer.size(); }

    // returned string_view is only valid until the next write call.
    [[nodiscard]] std::u8string_view string_view() const { return buffer; }

    // Takes the string out of the StringBuilder
    // You MUST NOT call `write()` after the string has been taken.
    [[nodiscard]] std::u8string&& takeString() { return std::move(buffer); }
};

}
