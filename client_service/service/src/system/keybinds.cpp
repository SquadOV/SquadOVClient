#include "system/keybinds.h"

namespace service::system {

Keybind Keybind::fromJson(const nlohmann::json& obj) {
    Keybind kb;

    for (const auto& ele: obj["keys"]) {
        kb.keys.push_back(ele.get<int>());
    }

    kb.mustHold = obj.value("mustHold", false);
    kb.holdSeconds = obj.value("holdSeconds", 0);
    return kb;
}

}