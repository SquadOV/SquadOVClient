#include "shared/strings/strings.h"

namespace shared::strings {

bool hasPrefix(std::string_view full, std::string_view prefix) {
    return full.substr(0, prefix.size()) == prefix;
}

}