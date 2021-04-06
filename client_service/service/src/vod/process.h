#pragma once

#include <filesystem>

namespace service::vod {

void processRawLocalVod(const std::filesystem::path& from, const std::filesystem::path& to);

}