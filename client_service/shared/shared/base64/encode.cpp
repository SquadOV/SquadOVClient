#include "shared/base64/encode.h"

#include <array>
#include <iostream>
#include <sstream>
#include <string_view>
#include <stdint.h>

namespace shared::base64 {
namespace {

const char BASE64_ENCODE_URL_CHARSET[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

std::string encodeBlock(std::string_view input) {
    // The input block can be anywhere between 1-3 characters long.
    // We should output a corresponding block that's between 2-4 characters long.
    // We use the left most 24 bits of the 32 bit uint32_t to track the bit wise representation
    // of the base64 characters.
    uint32_t block = 0;
    for (size_t i = 0; i < input.size(); ++i) {
        block |= (input[i] << (8 * (3 - i)));
    }

    const size_t numOutput = 4 - (3 - input.size());
    std::string ret;
    ret.reserve(numOutput);
    for (size_t i = 0; i < numOutput; ++i) {
        // Now we need to parse 6-bit chunks at a time.
        // To do this we do a bit operation to get the relevant 6 bits to be in the
        // least significant bits in a uint8_t and use that uint8_t to index into the
        // encoding charset.
        const uint8_t index = (block >> (6 * (4 - i) + 2)) & 0b00111111;
        ret.push_back(BASE64_ENCODE_URL_CHARSET[index]);
    }

    return ret;
}

}

std::string encode(const std::string& input) {
    // 3 character blocks get converted into 4 character base64 blocks.
    // If there's characters leftover at the end (the input size isn't divisible by 3) then we just convert the character(s)
    // that are there and skip padding the output.
    std::stringstream output;
    for (size_t i = 0; i < input.size(); i += 3) {
        output << encodeBlock(input.substr(i, 3));
    }
    return output.str();
}

}