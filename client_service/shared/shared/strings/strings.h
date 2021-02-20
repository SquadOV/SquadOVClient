#pragma once

#include <string>

namespace shared::strings {

bool hasPrefix(std::string_view full, std::string_view prefix);
static const std::string cEmptyString = "";

}