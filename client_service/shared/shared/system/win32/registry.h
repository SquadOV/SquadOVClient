#pragma once

#include <Windows.h>
#include <string>
#include <vector>

namespace shared::system::win32 {

void setRegistryKey(HKEY parentKey, const std::string& path, const std::string& key, const std::string& value);

}