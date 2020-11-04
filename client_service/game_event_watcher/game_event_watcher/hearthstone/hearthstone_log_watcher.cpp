#include "game_event_watcher/hearthstone/hearthstone_log_watcher.h"
#include "game_event_watcher/hearthstone/hearthstone_log_config.h"
#include "shared/filesystem/common_paths.h"

namespace fs = std::filesystem;

namespace game_event_watcher {

void HearthstoneLogWatcher::enableHearthstoneLogging()
{
    // Deliberate choice to make the Hearthstone local app data folder ahead of time
    // even if user doesn't have Hearthstone installed. This way, we're guaranteed to
    // work as soon as the user launches Hearthstone.
    const auto configPath = shared::filesystem::getHearthstoneAppDataFolder() / fs::path("log.config");
    if (!fs::exists(configPath)) {
        fs::create_directories(configPath.parent_path());
    }

    HearthstoneLogConfig cfg(configPath);
    cfg.enableLogSection(HearthstoneLogSection::Decks);
    cfg.enableLogSection(HearthstoneLogSection::Zone);
    cfg.enableLogSection(HearthstoneLogSection::Power);
    cfg.save();
}

HearthstoneLogWatcher::HearthstoneLogWatcher()
{
}

}