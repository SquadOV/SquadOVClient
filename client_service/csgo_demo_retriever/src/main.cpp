#include <boost/program_options.hpp>
#include <iostream>
#include <string>

#include "shared/time.h"
#include "shared/log/log.h"
#include "shared/base64/encode.h"
#include "shared/ipc/shared_memory.h"
#include "steamworks_interface/client.h"
#include "steamworks_interface/csgo/csgo_client.h"

#include "google/protobuf/util/json_util.h"

namespace po = boost::program_options;

// Returns a base64 encoded string
// DEMO_URL.DEMO_TIMESTAMP
std::string retrieveLatestSteamDemo(const shared::TimePoint& threshold) {
    steamworks_interface::csgo::CsgoClient client;
    client.performHandshake();

    std::optional<std::string> demoUrl;
    shared::TimePoint latestDemoTime = shared::zeroTime();

    const auto recentMatchList = client.getRecentCsgoMatchList(SteamUser()->GetSteamID().GetAccountID());
    for (const auto& match : recentMatchList.matches()) {
        const auto matchTime = shared::unixMsToTime(static_cast<int64_t>(match.matchtime()) * 1000);
        if (matchTime < threshold) {
            continue;
        }

        if (matchTime < latestDemoTime) {
            continue;
        }

        latestDemoTime = matchTime;

        // Need to look at roundstatsall and look for the round where 'map' is set.
        // 'map' (for whatever reason) is the URL of the demo we want.
        for (const auto& round : match.roundstatsall()) {
            if (round.has_map()) {
                demoUrl = round.map();
            }
        }
    }

    if (!demoUrl) {
        THROW_ERROR("No Demo URL found.");
    }

    return shared::base64::encode(demoUrl.value()) + "." + shared::base64::encode(shared::timeToIso(latestDemoTime));
}

int main(int argc, char** argv) {
    shared::log::Log::initializeGlobalLogger("csgo_demo_retriever.log");

    po::options_description desc("Options");
    desc.add_options()
        ("mode", po::value<std::string>()->default_value("steam"), "Where to fetch the demo from.")
        ("threshold", po::value<std::string>()->required(), "Time threshold. The demo must be from after this time.")
        ("shmem", po::value<std::string>(), "Shared memory identifer.")
        ("shvalue", po::value<std::string>(), "What segment to use in shared memory.");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    const auto mode = vm["mode"].as<std::string>();
    const auto threshold = shared::isoStrToTime(vm["threshold"].as<std::string>());
    LOG_INFO("Retrieving CSGO Demos: " << mode << " @ " << shared::timeToStr(threshold) << std::endl);

    std::string demoUrl;
    try {
        if (mode == "steam") {
            LOG_INFO("Using Steam." << std::endl);
            demoUrl = retrieveLatestSteamDemo(threshold);
        } else {
            LOG_ERROR("Invalid mode specified: " << mode << std::endl);
            return 1;
        }
    } catch (std::exception& ex) {
        LOG_ERROR("Failed to retrieve demo: " << ex.what() << std::endl);
        return 1;
    }
    LOG_INFO("Retrieved Demo URL: " << demoUrl << std::endl);
    if (vm.count("shmem") > 0 && vm.count("shvalue") > 0) {
        const auto memId = vm["shmem"].as<std::string>();
        const auto valueId = vm["shvalue"].as<std::string>();
        LOG_INFO("Shared Memory: " << memId << "\t" << valueId << std::endl);

        shared::ipc::SharedMemory shmem(memId);
        shmem.open();
        auto* outputStr = shmem.find<shared::ipc::SharedMemory::String>(valueId);
        *outputStr = shared::ipc::SharedMemory::String(demoUrl.begin(), demoUrl.end(), shmem.createAllocator<shared::ipc::SharedMemory::CharAllocator>());
    }
    return 0;
}