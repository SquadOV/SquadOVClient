#include "shared/system/win32/registry.h"
#include "shared/log/log.h"

namespace shared::system::win32 {

void setRegistryKey(HKEY parentKey, const std::string& path, const std::string& key, const std::string& value) {
    HKEY subkey;
    if (RegCreateKeyExA(
            parentKey,
            path.c_str(),
            0,
            NULL,
            0,
            KEY_ALL_ACCESS,
            nullptr,
            &subkey,
            nullptr
        ) != ERROR_SUCCESS
    ) {
        LOG_ERROR("Failed to set registry key: " << path << "[Key: " << key << " -- Value: " << value << "]" << std::endl);
        return;
    }
    RegSetValueExA(subkey, key.c_str(), 0, REG_EXPAND_SZ, (const BYTE*)value.c_str(), value.size() + 1);
}

}