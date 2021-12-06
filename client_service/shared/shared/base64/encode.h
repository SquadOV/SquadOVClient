#pragma once
#include <string>

namespace shared::base64 {

static const char BASE64_ENCODE_DEFAULT_CHARSET[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char BASE64_ENCODE_URL_CHARSET[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
std::string encode(const std::string& input, const char charset[65] = BASE64_ENCODE_URL_CHARSET);

}