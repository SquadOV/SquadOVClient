#pragma once

#include <string>

namespace shared::strings {

bool hasPrefix(std::string_view full, std::string_view prefix);
std::wstring utf8ToWcs(const std::string& str);
static const std::string cEmptyString = "";

}