#pragma once

#include "game_event_watcher/logs/base_log_watcher.h"
#include "game_event_watcher/logs/log_watcher.h"

#include <filesystem>

namespace game_event_watcher {

enum class ELeagueLogEvents {
    CommandLineCfg,
    LocalPlayer
};

struct LeagueCommandLineCfg {
    int64_t gameId;
    std::string region;
    std::string platformId;
};

struct LeagueLocalPlayer {
    std::string name;
    std::string puuid;
};

class LeagueLogWatcher: public BaseLogWatcher {
public:
    explicit LeagueLogWatcher(const shared::TimePoint& timeThreshold);

    void loadFromExecutable(const std::filesystem::path& exePath);
    void loadFromPath(const std::filesystem::path& logPath);
    void wait();
private:
    void onR3dChange(const LogLinesDelta& lines);

    std::filesystem::path _r3dPath;
    LogWatcherPtr _r3dWatcher;
};

using LeagueLogWatcherPtr = std::unique_ptr<LeagueLogWatcher>;

}