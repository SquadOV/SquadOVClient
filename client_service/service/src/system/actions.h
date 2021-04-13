#pragma once 
#include <functional>

namespace service::system {

enum class EAction {
    PushToTalkEnable,
    PushToTalkDisable
};

using ActionCallback = std::function<void()>;

}