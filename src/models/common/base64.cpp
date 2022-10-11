/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "base64.h"
#include "models/common/stringstream.h"
#include <cassert>

namespace UnTech::Base64 {

constexpr unsigned BLOCK_SIZE_BYTES = 3;
constexpr unsigned BLOCK_SIZE_CHARS = 4;

constexpr std::array<char8_t, 64> lookup = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '+', '/'
};

static inline std::array<char8_t, BLOCK_SIZE_CHARS> encodeBlock(std::span<const uint8_t, 3> s)
{
    return {
        lookup.at(((s[0] >> 2))),
        lookup.at(((s[0] & 0x03) << 4) | (s[1] >> 4)),
        lookup.at(((s[1] & 0x0F) << 2) | (s[2] >> 6)),
        lookup.at(((s[2] & 0x3F)))
    };
}

static inline std::array<char8_t, BLOCK_SIZE_CHARS> encodeBlock(std::span<const uint8_t, 2> s)
{
    return {
        lookup.at(((s[0] >> 2))),
        lookup.at(((s[0] & 0x03) << 4) | (s[1] >> 4)),
        lookup.at(((s[1] & 0x0F) << 2)),
        u8'=',
    };
}

static inline std::array<char8_t, BLOCK_SIZE_CHARS> encodeBlock(std::span<const uint8_t, 1> s)
{
    return {
        lookup.at(((s[0] >> 2))),
        lookup.at(((s[0] & 0x03) << 4)),
        u8'=',
        u8'='
    };
}

void encode(std::span<const uint8_t> data, StringStream& out, unsigned indent)
{
    constexpr unsigned CHARS_PER_LINE = 64;
    constexpr unsigned BLOCKS_PER_LINE = CHARS_PER_LINE / 4;

    std::u8string padding(indent, ' ');

    out.write(padding);

    unsigned blockId = 0;
    while (data.size() >= BLOCK_SIZE_BYTES) {
        if (blockId == BLOCKS_PER_LINE) {
            blockId = 0;
            out.write(u8"\n", padding);
        }
        blockId++;

        out.writeCharacters(encodeBlock(data.first<BLOCK_SIZE_BYTES>()));

        data = data.subspan(BLOCK_SIZE_BYTES);
    }

    if (!data.empty()) {
        if (blockId == BLOCKS_PER_LINE) {
            out.write(u8"\n", padding);
        }

        if (data.size() == 1) {
            out.writeCharacters(encodeBlock(data.first<1>()));
        }
        else if (data.size() == 2) {
            out.writeCharacters(encodeBlock(data.first<2>()));
        }
        static_assert(BLOCK_SIZE_BYTES == 3);
    }

    out.write(u8"\n");
}

inline uint8_t get_val(const char& c)
{
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    else if (c >= 'a' && c <= 'z') {
        return c - 'a' + 26;
    }
    else if (c >= '0' && c <= '9') {
        return c - '0' + 52;
    }
    else if (c == '+' || c == '-') {
        return 62;
    }
    else if (c == '/' || c == '_') {
        return 63;
    }
    else {
        return 0xFF;
    }
}

size_t decodeToBuffer(std::span<uint8_t> buffer, const std::u8string_view text)
{
    uint8_t token, tmp;

    // `bytesDecoded` can be larger then the size of the buffer
    size_t bytesDecoded = 0;
    auto bufferIt = buffer.begin();

    auto writeByte = [&](const uint8_t b) {
        if (bufferIt != buffer.end()) {
            *bufferIt++ = b;
        }
        bytesDecoded++;
    };

    auto textIt = text.begin();

#define NEXT_TOKEN()                \
    do {                            \
        if (textIt == text.end()) { \
            return bytesDecoded;    \
        }                           \
        token = get_val(*textIt++); \
    } while (token >= 64);

    while (true) {
        NEXT_TOKEN()
        tmp = token << 2;

        NEXT_TOKEN()
        writeByte(tmp | (token >> 4));
        tmp = (token & 0x0F) << 4;

        NEXT_TOKEN()
        writeByte(tmp | (token >> 2));
        tmp = (token & 0x03) << 6;

        NEXT_TOKEN()
        writeByte(tmp | token);
    }

    return bytesDecoded;
}

std::vector<uint8_t> decode(const std::u8string& text)
{
    std::vector<uint8_t> out(text.size() * 6 / 8 + 16);

    const size_t bytesDecoded = decodeToBuffer(out, text);
    assert(bytesDecoded <= out.size());

    out.resize(bytesDecoded);

    return out;
}

}
