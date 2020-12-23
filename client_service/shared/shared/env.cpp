#include "shared/env.h"

#include <cstdlib>

namespace shared {

std::string getEnv(const std::string& key, std::string default) {
#ifdef _WIN32
    char* data = nullptr;
    size_t len;
    _dupenv_s(&data, &len, key.c_str());
    if (!data || !len) {
        return default;
    }
    const auto ret = std::string(data);
    free(data);
    return ret;
#else
    return std::get_env(key.c_str());
#endif

}

#ifdef _WIN32
std::wstring getEnvW(const std::wstring& key, std::wstring default) {
    wchar_t* data = nullptr;
    size_t len;
    _wdupenv_s(&data, &len, key.c_str());
    if (!data || !len) {
        return default;
    }
    const auto ret = std::wstring(data);
    free(data);
    return ret;
}
#endif

}