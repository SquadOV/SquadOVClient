#pragma once

#include <optional>
#include <string>

namespace shared::system::keys {

std::optional<std::string> keycodeToName(int keycode);

}