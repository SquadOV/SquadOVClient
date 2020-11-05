#pragma once

#include "game_event_watcher/logs/base_log_watcher.h"
#include "game_event_watcher/logs/log_watcher.h"
#include "game_event_watcher/hearthstone/hearthstone_power_log_parser.h"
#include "game_event_watcher/hearthstone/hearthstone_raw_log.h"
#include <nlohmann/json.hpp>

namespace game_event_watcher {

enum class EHearthstoneLogEvents {
    GameReady,
    MatchStart,
    MatchEnd
};

struct HearthstoneGameConnectionInfo {
    std::string ip;
    int port;
    int gameId;
    int clientId;
    std::string spectateKey;
    bool reconnecting;

    nlohmann::json toJson() const;
};

using HearthstoneLogWatcherPtr = std::unique_ptr<class HearthstoneLogWatcher>;
// The hearthstone is just an amalgamation of a few different log watchers, namely:
// 1) The "default" log (log entries without a [XYZ] prefix).
// 2) The "Decks" log - one way to allow us to detect the current deck.
// 3) The "Power" log - the way for us to detect what happens in game.
class HearthstoneLogWatcher: public BaseLogWatcher {
public:
    static void enableHearthstoneLogging();

    HearthstoneLogWatcher(bool useTimeChecks);
    void loadFromExecutable(const std::filesystem::path& exePath);
    void loadFromFile(const std::filesystem::path& logFile);

    // The power parser maintains game state so we need to manually clear it when a game actually ends.
    void clearGameState();

    void wait();
private:
    void onLogChange(const LogLinesDelta& lines);

    LogWatcherPtr _watcher;

    HearthstonePowerLogParser _powerParser;
};

}