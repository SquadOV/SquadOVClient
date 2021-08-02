#include "game_event_watcher/wow/wow_log_watcher.h"
#include "shared/log/log.h"
#include "shared/time.h"
#include "shared/filesystem/common_paths.h"

#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace po = boost::program_options;
namespace fs = std::filesystem;
using namespace game_event_watcher;

int main(int argc, char** argv) {
    shared::log::Log::initializeGlobalLogger("wow_log_watcher.log");

    const auto tzDataFolder = shared::filesystem::getSquadOvTzDataFolder();
    date::set_install(tzDataFolder);
    date::get_tzdb_list();

    po::options_description desc("Options");
    desc.add_options()
        ("exe", po::value<std::string>()->required(), "WoW EXE to watch logs for.");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    game_event_watcher::WoWLogWatcher watcher(false, shared::nowUtc());
    watcher.notifyOnEvent(static_cast<int>(game_event_watcher::EWoWLogEvents::CombatLogLine), [](const shared::TimePoint& eventTime, const void* rawData){
        const auto* logLine = reinterpret_cast<const game_event_watcher::RawWoWCombatLog*>(rawData);
        std::cout << "DETECT WOW LOG LINE:" << logLine->rawLog << std::endl;
    });

    watcher.loadFromExecutable(fs::path(vm["exe"].as<std::string>()));
    watcher.wait();
    return 0;
}