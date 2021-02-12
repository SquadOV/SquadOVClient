#include "game_event_watcher/league/league_log_watcher.h"
#include "shared/log/log.h"
#include "shared/time.h"

#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace po = boost::program_options;
namespace fs = std::filesystem;
using namespace game_event_watcher;

int main(int argc, char** argv) {
    po::options_description desc("Options");
    desc.add_options()
        ("log", po::value<std::string>()->required(), "League Game log to parse.");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    game_event_watcher::LeagueLogWatcher watcher(shared::nowUtc());
    watcher.notifyOnEvent(static_cast<int>(game_event_watcher::ELeagueLogEvents::CommandLineCfg), [](const shared::TimePoint& eventTime, const void* rawData){
        std::cout << "DETECT CFG LINE" << std::endl;
        const auto* cfg = reinterpret_cast<const game_event_watcher::LeagueCommandLineCfg*>(rawData);
        if (!cfg) {
            LOG_WARNING("Null League Command Line Cfg?" << std::endl);
            return;
        }

        std::cout << "League Command Line Cfg: " << std::endl
            << "\tGame ID: " << cfg->gameId << std::endl
            << "\tRegion: " << cfg->region << std::endl
            << "\tPlatform: " << cfg->platformId << std::endl;
    });

    watcher.loadFromPath(fs::path(vm["log"].as<std::string>()));
    watcher.wait();
    
    return 0;
}