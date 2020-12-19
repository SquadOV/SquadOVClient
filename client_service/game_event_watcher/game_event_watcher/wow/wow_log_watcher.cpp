#include "game_event_watcher/wow/wow_log_watcher.h"

namespace fs = std::filesystem;
namespace game_event_watcher {

WoWLogWatcher::WoWLogWatcher(bool useTimeChecks, const shared::TimePoint& timeThreshold):
    BaseLogWatcher(useTimeChecks, timeThreshold) {

}

void WoWLogWatcher::loadFromExecutable(const fs::path& exePath) {
    const auto combatLogPath = exePath.parent_path() / fs::path("Logs") / fs::path("WoWCombatLog.txt");
    LOG_INFO("WoW Combat Log Path: " << combatLogPath.string() << std::endl);

    using std::placeholders::_1;
    _watcher = std::make_unique<LogWatcher>(combatLogPath, std::bind(&WoWLogWatcher::onCombatLogChange, this, _1), timeThreshold(), useTimeChecks());
    _watcher->disableBatching();
}

void WoWLogWatcher::onCombatLogChange(const LogLinesDelta& lines) {

}

}