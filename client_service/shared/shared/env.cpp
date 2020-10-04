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
    return std::string(data);
#else
    return std::get_env(key.c_str());
#endif

}

}