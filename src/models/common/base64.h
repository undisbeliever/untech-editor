/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace UnTech {
class StringStream;
}

namespace UnTech::Base64 {

/**
 * Encodes the given data as base64 text in the given file.
 * Uses MIME base64 style, indented by `indent` spaces.
 *
 * NOTE: `indent` will be capped if it is too large.
 */
void encode(std::span<const uint8_t> data, StringStream& out, unsigned indent = 0);

/**
 * Decodes the given text from base64 to binary.
 *
 * All invalid characters are skipped.
 */
std::vector<uint8_t> decode(const std::u8string& text);

// Returns the number of bytes decoded.
// DOES NOT the number of bytes written to buffer, may be larger than buffer.
size_t decodeToBuffer(std::span<uint8_t> buffer, const std::u8string_view text);

}
