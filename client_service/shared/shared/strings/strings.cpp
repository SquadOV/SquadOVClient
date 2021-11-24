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
    auto cStr = str.c_str();
    std::mbstate_t state = std::mbstate_t();
    const std::size_t len = 1 + std::mbsrtowcs(NULL, &cStr, 0, &state);
    std::vector<wchar_t> wstr(len);
    if (wstr.empty()) {
        return L"";
    }
    std::mbsrtowcs(&wstr[0], &cStr, wstr.size(), &state);
    return std::wstring(wstr.data(), wstr.size()-1);
}

std::string wcsToUtf8(const std::wstring& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(str);
}

}