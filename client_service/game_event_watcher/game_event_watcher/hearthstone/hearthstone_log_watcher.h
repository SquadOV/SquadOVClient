#pragma once

#include "game_event_watcher/logs/base_log_watcher.h"
#include "game_event_watcher/logs/log_watcher.h"
#include "game_event_watcher/hearthstone/hearthstone_power_log_parser.h"
#include "game_event_watcher/hearthstone/hearthstone_raw_log.h"
#include <nlohmann/json.hpp>

namespace game_event_watcher {

enum class EHearthstoneLogEvents {
    GameReady,
    // MatchConnect and MatchStart are two different events.
    // MatchConnect is when we're first notified that we are now going to start connecting to a game
    // while MatchStart is called when we've finally loaded into that game.
    // Similarly, MatchEnd is when the game finishes server side and MatchDisconnect is when the user is
    // logically disconnected from the game (i.e. their screen goes back to a non-game screen).
    MatchConnect,
    MatchStart,
    MatchEnd,
    MatchDisconnect,
    ArenaStartDraft,
    ArenaFinishDraft,
    ArenaDraftChoice
};

struct HearthstoneGameConnectionInfo {
    std::string ip;
    int port;
    int gameId;
    int clientId;
    std::string spectateKey;
    bool reconnecting;

    nlohmann::json toJson() const;
    bool valid() const { return !ip.empty(); }
};

using HearthstoneLogWatcherPtr = std::unique_ptr<class HearthstoneLogWatcher>;
// The hearthstone is just an amalgamation of a few different log watchers, namely:
// 1) The "default" log (log entries without a [XYZ] prefix).
// 2) The "Decks" log - one way to allow us to detect the current deck.
// 3) The "Power" log - the way for us to detect what happens in game.
class HearthstoneLogWatcher: public BaseLogWatcher {
public:
    static void enableHearthstoneLogging();

    HearthstoneLogWatcher(bool useTimeChecks, const shared::TimePoint& timeThreshold);
    void loadFromExecutable(const std::filesystem::path& exePath);

    void loadPrimaryFromFile(const std::filesystem::path& logFile);
    void loadPowerFromFile(const std::filesystem::path& logFile);
    void loadArenaFromFile(const std::filesystem::path& logFile);

    // The power parser maintains game state so we need to manually clear it when a game actually ends.
    void clearGameState();

    void wait();
private:
    void onPrimaryLogChange(const LogLinesDelta& lines);
    void onPowerLogChange(const LogLinesDelta& lines);
    void onArenaLogChange(const LogLinesDelta& lines);

    LogWatcherPtr _primaryWatcher;
    LogWatcherPtr _powerWatcher;
    LogWatcherPtr _arenaWatcher;

    HearthstonePowerLogParser _powerParser;
};

}