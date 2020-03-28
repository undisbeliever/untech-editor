/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "base64.h"
#include <sstream>
#include <stdexcept>

using namespace UnTech;

char lookup[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '+', '/'
};

void Base64::encode(const std::vector<uint8_t>& data, std::ostream& file, unsigned indent)
{
    const unsigned CHARS_PER_LINE = 64;
    const unsigned BLOCKS_PER_LINE = CHARS_PER_LINE / 4;

    for (unsigned u = 0; u < indent; u++) {
        file.put(' ');
    }

    const uint8_t* ptr = data.data();
    const uint8_t* endPtr = data.data() + data.size();

    uint8_t tmp0, tmp1, tmp2, tmp3;

    unsigned blocksOnLine = 0;
    while (ptr < endPtr) {
        {
            tmp0 = *ptr >> 2;
            tmp1 = (*ptr & 0x03) << 4;

            ptr++;
        }

        if (ptr < endPtr) {
            tmp1 |= *ptr >> 4;
            tmp2 = (*ptr & 0x0F) << 2;

            ptr++;
        }
        else {
            file.put(lookup[tmp0]);
            file.put(lookup[tmp1]);
            file.put('=');
            file.put('=');
            file.put('\n');
            break;
        }

        if (ptr < endPtr) {
            tmp2 |= *ptr >> 6;
            tmp3 = *ptr & 0x3F;

            ptr++;

            file.put(lookup[tmp0]);
            file.put(lookup[tmp1]);
            file.put(lookup[tmp2]);
            file.put(lookup[tmp3]);

            if (ptr < endPtr) {
                blocksOnLine++;
                if (blocksOnLine >= BLOCKS_PER_LINE) {
                    file.put('\n');
                    for (unsigned u = 0; u < indent; u++) {
                        file.put(' ');
                    }
                    blocksOnLine = 0;
                }
            }
            else {
                file.put('\n');
                break;
            }
        }
        else {
            file.put(lookup[tmp0]);
            file.put(lookup[tmp1]);
            file.put(lookup[tmp2]);
            file.put('=');
            file.put('\n');
            break;
        }
    }
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

std::vector<uint8_t> Base64::decode(const std::string& text)
{
    std::vector<uint8_t> out;

    uint8_t token, tmp;

    const char* ptr = text.c_str();
    const char* ptrEnd = text.c_str() + text.size();

#define NEXT_TOKEN()                              \
    if (ptr < ptrEnd) {                           \
        while ((token = get_val(*ptr++)) >= 64) { \
            if (ptr >= ptrEnd) {                  \
                return out;                       \
            }                                     \
        }                                         \
    }                                             \
    else {                                        \
        return out;                               \
    }

    while (ptr < ptrEnd) {
        NEXT_TOKEN()
        tmp = token << 2;

        NEXT_TOKEN()
        out.push_back(tmp | (token >> 4));
        tmp = (token & 0x0F) << 4;

        NEXT_TOKEN()
        out.push_back(tmp | (token >> 2));
        tmp = (token & 0x03) << 6;

        NEXT_TOKEN()
        out.push_back(tmp | token);
    }
    return out;
}
