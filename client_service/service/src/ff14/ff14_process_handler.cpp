#include "ff14/ff14_process_handler.h"
#include "shared/log/log.h"
#include "shared/games.h"
#include "shared/filesystem/utility.h"
#include "shared/filesystem/common_paths.h"
#include "process_watcher/games/act_process_detector.h"
#include "process_watcher/watcher.h"
#include "system/state.h"
#include "system/notification_hub.h"
#include "game_event_watcher/ff14/ff14_log_watcher.h"
#include "api/combat_log_client.h"
#include "api/squadov_api.h"

#include <boost/algorithm/string.hpp>
#include <rapidxml.hpp>

namespace fs = std::filesystem;

namespace service::ff14 {

class Ff14ProcessHandlerInstance {
public:
    explicit Ff14ProcessHandlerInstance(const process_watcher::process::Process& p);
    ~Ff14ProcessHandlerInstance();

    void cleanup();
    void forceStopRecording();
private:
    void loadGameVersion();
    void checkAndFindLogSetup();

    void onChangeZone(const shared::TimePoint& tm, const void* data);
    void onChangePrimaryPlayer(const shared::TimePoint& tm, const void* data);
    void onAddCombatant(const shared::TimePoint& tm, const void* data);
    void onCombatLogLine(const shared::TimePoint& tm, const void* data);

    process_watcher::process::Process _process;
    std::string _gameVersion;
    fs::path _networkLogDir;
    game_event_watcher::Ff14LogWatcherPtr _logWatcher;
    api::CombatLogClientPtr _combatLogClient;
};

Ff14ProcessHandlerInstance::Ff14ProcessHandlerInstance(const process_watcher::process::Process& p):
    _process(p)
{
    _combatLogClient = std::make_unique<api::CombatLogClient>(api::CombatLogEndpoint::Ff14);

    // Check FF14 game version. We'll store this information in conjunction with our logs since it might be
    // useful information later on (and maybe for parsing stuff as well just in case things change in the format).
    loadGameVersion();

    // We need to make sure the user has ACT with the FF14 plugin setup and running. We also need to make sure we can find the logs.
    checkAndFindLogSetup();

    // At this point we can start monitoring the log file for changes.
    _logWatcher = std::make_unique<game_event_watcher::Ff14LogWatcher>(true, shared::nowUtc());
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EFf14LogLineType::ChangeZone), std::bind(&Ff14ProcessHandlerInstance::onChangeZone, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EFf14LogLineType::ChangePrimaryPlayer), std::bind(&Ff14ProcessHandlerInstance::onChangePrimaryPlayer, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EFf14LogLineType::AddCombatant), std::bind(&Ff14ProcessHandlerInstance::onAddCombatant, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EFf14LogLineType::CombatLogLine), std::bind(&Ff14ProcessHandlerInstance::onCombatLogLine, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->loadFromParentFolder(_networkLogDir);

    service::api::getGlobalApi()->markUserAnalyticsEvent("launch_game");
    service::api::getGlobalApi()->markUserAnalyticsEvent("launch_ff14");
}

Ff14ProcessHandlerInstance::~Ff14ProcessHandlerInstance() {
}

void Ff14ProcessHandlerInstance::loadGameVersion() {
    const auto gameVerFilepath = _process.path().parent_path() / fs::path("ffxivgame.ver");
    if (!fs::exists(gameVerFilepath)) {
        LOG_WARNING("Failed to find FF14 game version at: " << gameVerFilepath << std::endl);
        return;
    }

    const auto version = boost::trim_copy(shared::filesystem::readTextFile(gameVerFilepath));
    LOG_INFO("Found FF14 Game Version: " << version << std::endl);
    _gameVersion = version;
}

void Ff14ProcessHandlerInstance::checkAndFindLogSetup() {
    // ACT's config files are found in %APPDATA%/Advanced Combat Tracker
    const fs::path actDir = shared::filesystem::getAppDataPath() / fs::path("Advanced Combat Tracker");
    if (!fs::exists(actDir)) {
        LOG_WARNING("ACT Config Directory not found: " << actDir << std::endl);
        DISPLAY_NOTIFICATION(
            service::system::NotificationSeverity::Error,
            service::system::NotificationDisplayType::NativeNotification,
            "SquadOV :: FF14 Configuration",
            "ACT is not installed or is not saving its configuration files to the expected folder: " << actDir
        );
        return;
    }

    // First check for the existence of the FF14 plugin.
    const fs::path ff14PluginPath = actDir / fs::path("Plugins") / fs::path("FFXIV_ACT_Plugin.dll");
    if (!fs::exists(ff14PluginPath)) {
        LOG_WARNING("FF14 ACT Plugin not installed: " << ff14PluginPath << std::endl);

        // We must actually return here. Sure the user *could* install the plugin at a later time
        // and start creating logs but they could configure it in a different location than what we're
        // expecting. Would rather just tell the user they fucked up and to fix their setup.
        DISPLAY_NOTIFICATION(
            service::system::NotificationSeverity::Error,
            service::system::NotificationDisplayType::NativeNotification,
            "SquadOV :: FF14 Configuration",
            "The FF14 plugin for ACT is not installed: " << ff14PluginPath
        );
        return;
    }

    // If the plugin exists then we'll try to read the *plugin* config file to get the location of the combat log.
    // If the file doesn't exist then it's safer here to assume that we can read the logs from the default location (hopefully).
    bool useDefaultPath = true;
    const fs::path ff14PluginConfigPath = actDir / fs::path("Config") / fs::path("FFXIV_ACT_Plugin.config.xml");
    if (fs::exists(ff14PluginConfigPath)) {
        auto xmlText = shared::filesystem::readTextFile(ff14PluginConfigPath);

        rapidxml::xml_document<> doc;
        doc.parse<0>(&xmlText[0]);

        // There's two settings we want to check
        //  1) Config > SettingsSerializer > CheckBox[Name=chkDisableCombatLog]
        //  2) Config > SettingsSerializer > TextBox[Name=txtLogFileDirectory]
        // If the combat log is disabled, warn the user to turn it on.
        // If the log file directory field exists, we us that as the network log directory.
        const auto* configNode = doc.first_node();
        if (configNode) {
            const auto* serializerNode = configNode->first_node();
            if (serializerNode) {
                for (auto* settingNode = serializerNode->first_node(); settingNode; settingNode = settingNode->next_sibling()) {
                    std::string name;
                    std::string value;

                    for (auto* attr = settingNode->first_attribute(); attr; attr = attr->next_attribute()) {
                        const std::string attrName(attr->name());
                        const std::string attrValue(attr->value());
                        
                        if (attrName == "Name") {
                            name = attrValue;
                        } else if (attrName == "Value") {
                            value = attrValue;
                        }
                    }

                    if (name == "chkDisableCombatLog" && value == "True") {
                        LOG_WARNING("FF14 Combat Logging Disabled in ACT!" << std::endl);
                        DISPLAY_NOTIFICATION(
                            service::system::NotificationSeverity::Warning,
                            service::system::NotificationDisplayType::NativeNotification,
                            "SquadOV :: FF14 Configuration",
                            "You have disabled combat logging for FF14 in ACT! SquadOV will not record."
                        );
                    } else if (name == "txtLogFileDirectory") {
                        useDefaultPath = false;
                        _networkLogDir = fs::path(value);
                    }
                }
            }
        }
    }
    
    if (useDefaultPath || !fs::exists(_networkLogDir)) {
        LOG_WARNING("FF14 Plugin Config Path not found...assuming default." << std::endl);
        _networkLogDir = actDir / fs::path("FFXIVLogs");
    }

    // Finally check that ACT is actually running. If not, we should warn the user.
    process_watcher::games::ActProcessDetector detector;
    if (!process_watcher::isProcessRunning(detector)) {
        LOG_WARNING("ACT is not currently running at the time of FF14 launch." << std::endl);
        DISPLAY_NOTIFICATION(
            service::system::NotificationSeverity::Warning,
            service::system::NotificationDisplayType::NativeNotification,
            "SquadOV :: FF14 Configuration",
            "ACT is not currently running. SquadOV will not record FF14 unless combat logs are produced."
        );
    }

    LOG_INFO("FF14 ACT Log Path: " << _networkLogDir << std::endl);
}

void Ff14ProcessHandlerInstance::onChangeZone(const shared::TimePoint& tm, const void* data) {
    // Check if we're entering a relevant zone that we potentially wish to record.
}

void Ff14ProcessHandlerInstance::onChangePrimaryPlayer(const shared::TimePoint& tm, const void* data) {

}

void Ff14ProcessHandlerInstance::onAddCombatant(const shared::TimePoint& tm, const void* data) {

}

void Ff14ProcessHandlerInstance::onCombatLogLine(const shared::TimePoint& tm, const void* data) {
    // Batch combat log line data and send to server for processing.
    const game_event_watcher::LogLinesDelta* logLines = reinterpret_cast<const game_event_watcher::LogLinesDelta*>(data);
    for (const auto& ln: *logLines) {
        _combatLogClient->addLine(ln);
    }
}

void Ff14ProcessHandlerInstance::cleanup() {

}

void Ff14ProcessHandlerInstance::forceStopRecording() {

}

Ff14ProcessHandler::Ff14ProcessHandler() = default;
Ff14ProcessHandler::~Ff14ProcessHandler() = default;


void Ff14ProcessHandler::forceStopRecording() {
    if (!_instance) {
        return;
    }
    LOG_INFO("Force Stop Recording: FF14" << std::endl);
    _instance->forceStopRecording();
}

void Ff14ProcessHandler::onProcessStarts(const process_watcher::process::Process& p) {
    if (_instance) {
        return;
    }

    LOG_INFO("START FF14" << std::endl);
    _instance = std::make_unique<Ff14ProcessHandlerInstance>(p);
    service::system::getGlobalState()->markGameRunning(shared::EGame::Ff14, true);
}

void Ff14ProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    LOG_INFO("STOP FF14" << std::endl);
    service::system::getGlobalState()->markGameRunning(shared::EGame::Ff14, false);
    _instance->cleanup();
    _instance.reset(nullptr);
    LOG_INFO("\tFF14 fully stopped." << std::endl);
}

}