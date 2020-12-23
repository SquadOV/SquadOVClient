#pragma once

#include <string>

namespace shared {

std::string getEnv(const std::string& key, std::string default = "");

#ifdef _WIN32
std::wstring getEnvW(const std::wstring& key, std::wstring default = std::wstring(L""));
#endif
}