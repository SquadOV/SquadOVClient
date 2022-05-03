#pragma once

#include <vector>
#include <nlohmann/json.hpp>

namespace service::system {

struct Keybind {
    std::vector<int> keys;
    bool mustHold = false;
    int holdSeconds = 0;

    static Keybind fromJson(const nlohmann::json& obj);
};

enum class EKeybindId {
    Ptt1,
    Ptt2,
    Clip,
    Bookmark
};

}