#include "shared/strings/strings.h"
#include "shared/errors/error.h"

#include <codecvt>
#include <cwchar>
#include <vector>

namespace shared::strings {

bool hasPrefix(std::string_view full, std::string_view prefix) {
    return full.substr(0, prefix.size()) == prefix;
}

std::wstring utf8ToWcs(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(str);
}

std::string wcsToUtf8(const std::wstring& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(str);
}

}