#include "game_event_watcher/wow/wow_log_watcher.h"
#include "shared/log/log.h"
#include "shared/time.h"
#include "shared/filesystem/common_paths.h"
#include "api/combat_log_client.h"

#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace po = boost::program_options;
namespace fs = std::filesystem;
using namespace game_event_watcher;

int main(int argc, char** argv) {
    shared::log::Log::initializeGlobalLogger("combat_log_upload_tester.log");

    const auto tzDataFolder = shared::filesystem::getSquadOvTzDataFolder();
    date::set_install(tzDataFolder);
    date::get_tzdb_list();

    po::options_description desc("Options");
    desc.add_options()
        ("log", po::value<std::string>()->required(), "Combat log to upload.");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    service::api::CombatLogClient client(service::api::CombatLogEndpoint::Ff14);
    client.start();

    std::ifstream file(fs::path(vm["log"].as<std::string>());
    std::string line;
    while (file.is_open() && std::getline(file, line)) {
        client.addLine(line);
    }

    client.flush();
    return 0;
}