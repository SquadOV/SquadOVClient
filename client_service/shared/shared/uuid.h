#pragma once

#include <string>

namespace shared {

std::string generateUuidv4();
bool isValidUuid(const std::string& test);

}