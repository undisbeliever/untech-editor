/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "stringbuilder.h"
#include <array>
#include <charconv>

namespace UnTech::StringBuilder {

template <typename T>
static void concat_int(std::u8string& s, const T value)
{
    static_assert(sizeof(char8_t) == sizeof(char));

    std::array<char, 32> buffer{};

    auto r = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);

    if (r.ec == std::errc()) {
        s.append(buffer.data(), r.ptr);
    }
    else {
        abort();
    }
}

void concat(std::u8string& s, int32_t value) { concat_int(s, value); }
void concat(std::u8string& s, int64_t value) { concat_int(s, value); }
void concat(std::u8string& s, uint32_t value) { concat_int(s, value); }
void concat(std::u8string& s, uint64_t value) { concat_int(s, value); }

//  UnTech::StringBuilder::hex<0> has no padding
void concat(std::u8string& s, const hex<0> value)
{
    static_assert(std::is_same_v<decltype(value.value), uint32_t>);
    static_assert(sizeof(char8_t) == sizeof(char));

    std::array<char, 8> buffer{};

    auto r = std::to_chars(buffer.data(), buffer.data() + buffer.size(),
                           value.value, 16);

    if (r.ec == std::errc()) {
        s.append(buffer.data(), r.ptr);
    }
    else {
        abort();
    }
}

// UnTech::StringBuilder::hex<N> has padding
template <unsigned N>
void concat(std::u8string& s, const hex<N> value)
{
    static_assert(std::is_same_v<decltype(value.value), uint32_t>);
    static_assert(N == stringSize(hex<N>(uint32_t(0))));

    std::array<char8_t, N> buffer{};

    for (size_t i = 0; i < N; i++) {
        const auto s = (N - i - 1) * 4;
        const auto digit = (value.value >> s) % 16;

        buffer.at(i) = digit < 10 ? digit + u8'0'
                                  : digit + u8'a' - 10;
    }

    s.append(buffer.begin(), buffer.end());
}

template void concat(std::u8string&, const hex<4>);
template void concat(std::u8string&, const hex<6>);
template void concat(std::u8string&, const hex<8>);

}
