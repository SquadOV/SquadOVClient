#include "shared/base64/decode.h"
#include "shared/base64/encode.h"

#include <sstream>
#include <string_view>
#include <unordered_map>

namespace shared::base64 {
namespace {

uint8_t base64CharToValue(char c) {
    static std::unordered_map<char, uint8_t> reverseMap = [](){
        std::unordered_map<char, uint8_t> map;
        for (uint8_t i = 0; i < 64; ++i) {
            map[BASE64_ENCODE_URL_CHARSET[i]] = i;
        }
        return map;
    }();
    return reverseMap[c];
}

std::string decodeBlock(std::string_view input) {
    if (input.size() <= 1) {
        return "";
    }

    // We may have anywhere between 2-4 input characters.
    // This means the input contains 12-24 bits of information.
    // This will result in 1-3 output characters where each output
    // character is 8 bits.
    uint32_t block = 0;
    for (size_t i = 0; i < input.size(); ++i) {
        block |= (base64CharToValue(input[i]) << (6 * (4 - i) + 2));
    }

    const size_t numOutput = 3 - (4 - input.size());
    std::string ret;
    ret.reserve(numOutput);
    for (size_t i = 0; i < numOutput; ++i) {
        const char nextChar = (block >> (8 * (3 - i))) & 0xFF;
        ret.push_back(nextChar);
    }
    return ret;
}

}

std::string decode(const std::string& input) {
    // 4 character blocks get decoded into 3 character blocks.
    std::stringstream output;
    for (size_t i = 0; i < input.size(); i += 4) {
        output << decodeBlock(input.substr(i, 4));
    }
    return output.str();
}

}