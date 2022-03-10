#include "game_event_watcher/ff14/ff14_log_watcher.h"
#include "shared/filesystem/utility.h"
#include <boost/tokenizer.hpp>

using namespace std::literals::string_literals;
namespace fs = std::filesystem;
namespace game_event_watcher {
namespace {

bool parseCombatLogLine(const std::string& raw, Ff14LogLine& parsed) {
    boost::char_separator<char> sep("|");
    boost::tokenizer tok(raw, sep);
    std::copy(tok.begin(), tok.end(), std::back_inserter(parsed.parts));

    // Part 0: Event type
    const auto iType = std::atoi(parsed.parts[0].c_str());
    if (iType != static_cast<int>(EFf14LogLineType::ChangeZone) ||
        iType != static_cast<int>(EFf14LogLineType::ChangePrimaryPlayer) ||
        iType != static_cast<int>(EFf14LogLineType::AddCombatant)) {
        return false;
    }
    parsed.type = static_cast<EFf14LogLineType>(iType);

    // Part 1: Timestamp in local time (seems to be ISO8601 with timezone)
    parsed.tm = shared::isoStrWithTimeZoneToTime(parsed.parts[1]);

    // The rest of the parts is dependent on the event.
    if (parsed.parts.size() > 2) {
        parsed.parts.erase(parsed.parts.begin(), parsed.parts.begin() + 2);
    } else {
        parsed.parts.clear();
    }

    return true;
}

}

Ff14ParsedData Ff14LogLine::parse() const {
    Ff14ParsedData data;
    if (type == EFf14LogLineType::ChangeZone) {
        Ff14ChangeZoneData subdata;
        subdata.id = std::stoll(parts[0], nullptr, 16);
        subdata.name = parts[1];
        data = subdata;
    } else if (type == EFf14LogLineType::ChangePrimaryPlayer) {
        Ff14ChangePrimaryPlayerData subdata;
        subdata.id = std::stoll(parts[0], nullptr, 16);
        subdata.name = parts[1];
        data = subdata;
    } else if (type == EFf14LogLineType::AddCombatant) {
        Ff14AddCombatantData subdata;
        subdata.id = std::stoll(parts[0], nullptr, 16);
        subdata.name = parts[1];
        subdata.job = std::stoll(parts[2], nullptr, 16);
        subdata.level = std::stoll(parts[3], nullptr, 16);
        subdata.owner = std::stoll(parts[4], nullptr, 16);
        subdata.worldId = std::stoll(parts[5], nullptr, 16);
        subdata.world = parts[6];
        subdata.npcNameId = std::stoll(parts[6], nullptr, 16);
        subdata.npcBaseId = std::stoll(parts[7], nullptr, 16);
        data = subdata;
    }
    return data;
}

Ff14LogWatcher::Ff14LogWatcher(bool useTimeChecks, const shared::TimePoint& timeThreshold):
    BaseLogWatcher(useTimeChecks, timeThreshold)
{
}

Ff14LogWatcher::~Ff14LogWatcher() {
    _running = false;
    if (_timestampLogThread.joinable()) {
        _timestampLogThread.join();
    }
}

void Ff14LogWatcher::loadFromParentFolder(const fs::path& logDir) {
    // TODO: Is there a way to consolidate this code with the code in the wow log watcher?
    _timestampLogThread = std::thread([this, logDir](){
        std::filesystem::path logFile;
        std::optional<typename std::ifstream::pos_type> position = std::nullopt;

        while (_running) {
            try {
                if (!fs::exists(logFile) || (shared::filesystem::timeOfLastFileWrite(logFile) < timeThreshold()) || (shared::filesystem::timeOfLastFileWrite(logFile) < _lastLogTime)) {
                    const auto oldLogFile = logFile;
                    logFile = shared::filesystem::getNewestFileInFolder(logDir, [](const fs::path& path){
                        if (path.extension() != ".log") {
                            return false;
                        }

                        if (path.filename().native().find(L"Network_"s) != 0) {
                            return false;
                        }

                        return true;
                    });

                    try {
                        if (fs::exists(logFile)) {
                            std::ifstream file(logFile);
                            if (file.is_open()) {
                                file.seekg(0, std::ios_base::end);
                                position = file.tellg();
                            }
                            file.close();
                        }
                    } catch (std::exception& ex) {
                        LOG_WARNING("Failed to get log file position: " << ex.what() << std::endl);
                    }
                } else {
                    const auto lastWriteTime = shared::filesystem::timeOfLastFileWrite(logFile);
                    if (_running && lastWriteTime >= timeThreshold() && lastWriteTime >= _lastLogTime && fs::exists(logFile)) {
                        _lastLogTime = lastWriteTime;
                        loadFromPath(logFile, true, position);
                    }
                }
            } catch (std::exception& ex) {
                LOG_WARNING("Exception when detecting FF14 combat log: " << ex.what() << std::endl);
                continue;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

void Ff14LogWatcher::loadFromPath(const std::filesystem::path& combatLogPath, bool loop, std::optional<typename std::ifstream::pos_type> position) {
    if (_logPath == combatLogPath) {
        return;
    }

    _logPath = combatLogPath;
    _logLine = 0;
    LOG_INFO("Load FF14 Combat Log File: " << _logPath << std::endl);
    _watcher = std::make_unique<LogWatcher>(
        _logPath,
        std::bind(&Ff14LogWatcher::onCombatLogChange, this, std::placeholders::_1),
        timeThreshold(),
        false,
        false,
        loop,
        position
    );
    _watcher->disableBatching();
}

void Ff14LogWatcher::onCombatLogChange(const LogLinesDelta& lines) {
    for (const auto& ln : lines) {
        if (ln.empty()) {
            continue;
        }

        Ff14LogLine log;
        const bool parsed = parseCombatLogLine(ln, log);

        if (parsed) {
            // Note that this is just a small subset of the possible types.
            // We just don't really care about the rest on the client side of things at least.
            const auto parsedData = log.parse();
            if (log.type == EFf14LogLineType::ChangeZone) {
                notify(static_cast<int>(log.type), log.tm, (void*)&std::get<Ff14ChangeZoneData>(parsedData));
            } else if (log.type == EFf14LogLineType::ChangePrimaryPlayer) {
                notify(static_cast<int>(log.type), log.tm, (void*)&std::get<Ff14ChangePrimaryPlayerData>(parsedData));
            } else if (log.type == EFf14LogLineType::AddCombatant) {
                notify(static_cast<int>(log.type), log.tm, (void*)&std::get<Ff14AddCombatantData>(parsedData));
            }
        }

        // We pass on the raw data on as well for additional parsing on our servers. This has to come AFTER
        // notification of other events since we want to be able to selectively send up combat log lines for
        // when the user is running some content we wish to actually record.
        notify(static_cast<int>(EFf14LogLineType::CombatLogLine), shared::nowUtc(), (void*)&lines, false);

    }
}

}