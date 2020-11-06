#include "game_event_watcher/hearthstone/hearthstone_log_watcher.h"
#include "shared/log/log.h"

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
        ("log", po::value<std::string>()->required(), "Hearthstone log to parse.")
        ("power", po::value<std::string>()->required(), "Hearthstone power log to parse.")
        ("output", po::value<std::string>()->required(), "Output of the JSON power log to send to the server.");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    const fs::path logPath(vm["log"].as<std::string>());
    const fs::path powerPath(vm["power"].as<std::string>());

    HearthstoneLogWatcher watcher(false);
    watcher.notifyOnEvent(static_cast<int>(EHearthstoneLogEvents::GameReady), [](const shared::TimePoint& tm, const void* data){
        LOG_INFO("Hearthstone Game Ready [" << shared::timeToStr(tm) << "]" << std::endl);
    });

    watcher.notifyOnEvent(static_cast<int>(EHearthstoneLogEvents::MatchStart), [](const shared::TimePoint& tm, const void* data){
        LOG_INFO("Hearthstone Game Start [" << shared::timeToStr(tm) << "]" << std::endl);
    });

    watcher.notifyOnEvent(static_cast<int>(EHearthstoneLogEvents::MatchConnect), [](const shared::TimePoint& tm, const void* data){
        const HearthstoneGameConnectionInfo* info = reinterpret_cast<const HearthstoneGameConnectionInfo*>(data);
        LOG_INFO("Hearthstone Game Connect [" << shared::timeToStr(tm) << "]" << std::endl
            << "\tGame Server:" << info->ip << ":" << info->port << std::endl
            << "\tGame ID: " << info->gameId << std::endl
            << "\tClient ID: " << info->clientId << std::endl
            << "\tSpectate Key: " << info->spectateKey << std::endl
            << "\tReconnecting: " << info->reconnecting << std::endl
        );
    });

    watcher.notifyOnEvent(static_cast<int>(EHearthstoneLogEvents::MatchEnd), [&watcher, &vm](const shared::TimePoint& tm, const void* data){
        LOG_INFO("Hearthstone Game End [" << shared::timeToStr(tm) << "]" << std::endl);
        const nlohmann::json* j = reinterpret_cast<const nlohmann::json*>(data);
        std::ofstream f(vm["output"].as<std::string>());
        f << j->dump(4);
        f.close();

        watcher.clearGameState();
    });
    watcher.loadPrimaryFromFile(logPath);
    watcher.loadPowerFromFile(powerPath);
    watcher.wait();
    
    return 0;
}