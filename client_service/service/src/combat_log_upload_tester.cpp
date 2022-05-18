#include "shared/log/log.h"
#include "shared/time.h"
#include "shared/filesystem/common_paths.h"
#include "shared/env.h"
#include "api/combat_log_client.h"
#include "api/squadov_api.h"
#include "game_event_watcher/wow/wow_log_watcher.h"

#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace po = boost::program_options;
namespace fs = std::filesystem;

int main(int argc, char** argv) {
    shared::log::Log::initializeGlobalLogger("combat_log_upload_tester.log");

    const auto tzDataFolder = shared::filesystem::getSquadOvTzDataFolder();
    date::set_install(tzDataFolder);
    date::get_tzdb_list();

    service::api::getGlobalApi()->setSessionId(shared::getEnv("SQUADOV_SESSION_ID"));

    po::options_description desc("Options");
    desc.add_options()
        ("game", po::value<std::string>()->required(), "Game prefix.")
        ("log", po::value<std::string>()->required(), "Combat log to upload.");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    const auto game = vm["game"].as<std::string>();
    service::api::CombatLogClient client(
        (game == "wow") ? service::api::CombatLogEndpoint::Wow :
            service::api::CombatLogEndpoint::Ff14);
    if (game == "wow") {
        game_event_watcher::WoWCombatLogState state;
        state.advancedLog = true;
        state.buildVersion = "9.0.2";
        state.combatLogVersion = "17";
        client.setCombatLogState(state.toJson());
    }
    client.setPartitionId(game + "_boba", shared::nowUtc());
    client.start();

    const auto filePath = fs::path(vm["log"].as<std::string>());
    std::ifstream file(filePath);
    std::string line;
    while (file.is_open() && std::getline(file, line)) {
        if (game == "wow") {
            game_event_watcher::RawWoWCombatLog log;
            if (game_event_watcher::parseRawCombatLogLine(line, log, filePath)) {
                client.addLine(log.toJson().dump());
            }
        } else {
            client.addLine(line);
        }
    }

    client.flush();
    return 0;
}