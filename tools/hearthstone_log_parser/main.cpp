#include "game_event_watcher/hearthstone/hearthstone_log_watcher.h"
#include "shared/log/log.h"
#include "shared/time.h"

#include <boost/program_options.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
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

    HearthstoneLogWatcher watcher(false, shared::nowUtc());
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
        const auto fname = vm["output"].as<std::string>();
        const auto jsonBody = j->dump(4);
        std::ofstream f(fname);
        f << jsonBody;
        f.close();

        std::vector<char> gzipBuffer;
        auto gzipDevice = boost::iostreams::back_inserter(gzipBuffer);
        boost::iostreams::filtering_ostream outputStream;
        outputStream.push(boost::iostreams::gzip_compressor{});
        outputStream.push(gzipDevice);
        outputStream << jsonBody << std::flush;
        outputStream.pop();

        std::ofstream g(fname + ".gz", std::ios_base::binary);
        g.write(gzipBuffer.data(), gzipBuffer.size());
        g.close();

        watcher.clearGameState();
    });

    watcher.notifyOnEvent(static_cast<int>(EHearthstoneLogEvents::MatchDisconnect), [](const shared::TimePoint& tm, const void* data){
        LOG_INFO("Hearthstone Game Disconnect [" << shared::timeToStr(tm) << "]" << std::endl);
    });

    watcher.loadPrimaryFromFile(logPath);
    watcher.loadPowerFromFile(powerPath);
    watcher.wait();
    
    return 0;
}