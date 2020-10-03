#include "shared/env.h"

#include <cstdlib>

namespace shared {

std::string getEnv(const std::string& key) {
#ifdef _WIN32
    char* data;
    size_t len;
    _dupenv_s(&data, &len, key.c_str());
    return std::string(data);
#else
    return std::get_env(key.c_str());
#endif

}

}