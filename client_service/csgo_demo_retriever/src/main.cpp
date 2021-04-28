#include <boost/program_options.hpp>
#include <iostream>
#include <string>

#include "shared/time.h"
#include "shared/log/log.h"
#include "steamworks_interface/client.h"
#include "steamworks_interface/csgo/csgo_client.h"

namespace po = boost::program_options;

std::string retrieveLatestSteamDemo(const shared::TimePoint& threshold) {
    steamworks_interface::csgo::CsgoClient client;
    client.waitForConnect();
    return "";
}

int main(int argc, char** argv) {
    shared::log::Log::initializeGlobalLogger("csgo_demo_retriever.log");

    po::options_description desc("Options");
    desc.add_options()
        ("mode", po::value<std::string>()->default_value("steam"), "Where to fetch the demo from.")
        ("threshold", po::value<std::string>()->required(), "Time threshold. The demo must be from after this time.");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    const auto mode = vm["mode"].as<std::string>();
    const auto threshold = shared::isoStrToTime(vm["threshold"].as<std::string>());
    LOG_INFO("Retrieving CSGO Demos: " << mode << " @ " << shared::timeToStr(threshold) << std::endl);
    std::string demoUrl;
    try {
        if (mode == "steam") {
            demoUrl = retrieveLatestSteamDemo(threshold);
        } else {
            LOG_ERROR("Invalid mode specified: " << mode << std::endl);
            return 1;
        }
    } catch (std::exception& ex) {
        LOG_ERROR("Failed to retrieve demo: " << ex.what() << std::endl);
        return 1;
    }

    return 0;
}