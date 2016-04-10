#ifndef _UNTECH_MODELS_COMMON_BASE64_H_
#define _UNTECH_MODELS_COMMON_BASE64_H_

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
#endif
