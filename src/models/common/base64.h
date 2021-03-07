/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace UnTech {
namespace Base64 {

/**
 * Encodes the given data as base64 text in the given file.
 * Uses MIME base64 style, indented by `indent` spaces.
 */
void encode(const uint8_t* ptr, const size_t size, std::ostream& file, unsigned indent);
void encode(const std::vector<uint8_t>& data, std::ostream& file, unsigned indent = 0);

template <size_t N>
void encode(const std::array<uint8_t, N>& data, std::ostream& file, unsigned indent)
{
    _encode(data.data(), data.size(), file, indent);
}

/**
 * Decodes the given text from base64 to binary.
 *
 * All invalid characters are skipped.
 */
std::vector<uint8_t> decode(const std::string& text);

// Returns the number of bytes decoded.
// DOES NOT the number of bytes written to buffer, may be larger than buffer.
size_t decodeToBuffer(uint8_t* buffer, const size_t bufferSize, const std::string& text);

}
}
