/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace UnTech {
namespace Base64 {

/**
 * Encodes the given data as base64 text in the given file.
 * Uses MIME base64 style, indented by `indent` spaces.
 */
void encode(const std::vector<uint8_t>& data, std::ostream& file, unsigned indent = 0);

/**
 * Decodes the given text from base64 to binary.
 *
 * All invalid characters are skipped.
 */
std::vector<uint8_t> decode(const std::string& text);
}
}
