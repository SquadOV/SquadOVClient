#pragma once 
#include <functional>

namespace service::system {

enum class EAction {
    PushToTalkEnable,
    PushToTalkDisable,
    Bookmark
};

using ActionCallback = std::function<void()>;

}