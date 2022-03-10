#pragma once

#include "game_event_watcher/logs/base_log_watcher.h"
#include "game_event_watcher/logs/log_watcher.h"
#include "shared/time.h"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <deque>
#include <variant>

namespace game_event_watcher {

enum class EFf14LogLineType {
    ChangeZone = 1,
    ChangePrimaryPlayer = 2,
    AddCombatant = 3,
    CombatLogLine = 500
};

struct Ff14ChangeZoneData {
    int64_t id;
    std::string name;
};

struct Ff14ChangePrimaryPlayerData {
    int64_t id;
    std::string name;
};

struct Ff14AddCombatantData {
    std::string id;
    std::string name;
    int64_t job;
    int64_t level;
    std::string owner;
    int64_t worldId;
    std::string world;
    std::string npcNameId;
    std::string npcBaseId;
};

using Ff14ParsedData = std::variant<
    std::monostate,
    Ff14ChangeZoneData,
    Ff14ChangePrimaryPlayerData,
    Ff14AddCombatantData
>;

struct Ff14LogLine {
    EFf14LogLineType type;
    shared::TimePoint tm;
    std::deque<std::string> parts;

    Ff14ParsedData parse() const;
};

class Ff14LogWatcher : public BaseLogWatcher {
public:
    Ff14LogWatcher(bool useTimeChecks, const shared::TimePoint& timeThreshold);
    ~Ff14LogWatcher();
    
    void loadFromParentFolder(const std::filesystem::path& logDir);
    void loadFromPath(const std::filesystem::path& logPath, bool loop = true, std::optional<typename std::ifstream::pos_type> position = std::nullopt);

private:
    void onCombatLogChange(const LogLinesDelta& lines);

    std::filesystem::path _logPath;
    LogWatcherPtr _watcher;
    int64_t _logLine = 0;
    bool _running = true;
    std::thread _timestampLogThread;
    shared::TimePoint _lastLogTime = shared::zeroTime();
};

using Ff14LogWatcherPtr = std::unique_ptr<Ff14LogWatcher>;

}