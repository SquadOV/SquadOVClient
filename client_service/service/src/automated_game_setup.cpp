#include "shared/log/log.h"
#include "system/settings.h"
#include "shared/system/install.h"
#include "shared/filesystem/common_paths.h"
#include "shared/json.h"
#include "api/squadov_api.h"
#include "shared/env.h"
#include "recorder/pipe/cloud_storage_piper.h"
#include "recorder/pipe/pipe_client.h"
#include "recorder/pipe/pipe.h"
#include "shared/http/http_client.h"
#include "shared/uuid.h"
#include "uploader/uploader.h"
#include <vector>

#include <boost/program_options.hpp>
#include <optional>
#include <fstream>

namespace fs = std::filesystem;
namespace po = boost::program_options;

struct SpeedCheckReport {
    double standardUploadMbps = 0.0;
};

struct WowSetupReport {
    bool hasClAddon = false;
    bool hasRecommendedAddon = false;
    bool hasMultipleAddons = false;
    bool installedAddon = false;
    bool turnedOnAdvanced = false;
    std::vector<std::string> addons;
};

struct SetupReport {
    bool needsAttention = false;
    bool firstTime = false;
    std::optional<SpeedCheckReport> speedCheck;
    std::optional<WowSetupReport> wowRetail;
    std::optional<WowSetupReport> wowClassic;
    std::optional<WowSetupReport> wowTbc;
    bool needsHwCheck = false;
};

namespace shared::json {

template<>
struct JsonConverter<SpeedCheckReport> {
    static nlohmann::json to(const SpeedCheckReport& v) {
        nlohmann::json o = { 
            { "standardUploadMbps", v.standardUploadMbps }
        };
        return o;
    }
};

template<>
struct JsonConverter<WowSetupReport> {
    static nlohmann::json to(const WowSetupReport& v) {
        nlohmann::json o = {
            { "hasClAddon", v.hasClAddon },
            { "hasRecommendedAddon", v.hasRecommendedAddon },
            { "hasMultipleAddons", v.hasMultipleAddons },
            { "installedAddon", v.installedAddon },
            { "turnedOnAdvanced", v.turnedOnAdvanced },
            { "addons", shared::json::JsonConverter<decltype(v.addons)>::to(v.addons) }
        };
        return o;
    }
};

template<>
struct JsonConverter<SetupReport> {
    static nlohmann::json to(const SetupReport& v) {
        nlohmann::json o = {
            { "needsAttention", v.needsAttention },
            { "firstTime", v.firstTime },
            { "needsHwCheck", v.needsHwCheck }
        };

        if (v.speedCheck) {
            o["speedCheck"] = JsonConverter<SpeedCheckReport>::to(v.speedCheck.value());
        }

        if (v.wowRetail) {
            o["wowRetail"] = JsonConverter<WowSetupReport>::to(v.wowRetail.value());
        }

        if (v.wowClassic) {
            o["wowClassic"] = JsonConverter<WowSetupReport>::to(v.wowClassic.value());
        }

        if (v.wowTbc) {
            o["wowTbc"] = JsonConverter<WowSetupReport>::to(v.wowTbc.value());
        }

        return o;
    }
};

}

std::optional<SpeedCheckReport> generateSpeedCheckReport() {
    SpeedCheckReport report;

    try {
        // Note that setSessionId also does an API call to pull the current user.
        service::api::getGlobalApi()->setSessionId(shared::getEnv("SQUADOV_SESSION_ID", ""));
    } catch (std::exception& ex) {
        // This needs to be caught because the session could be completely invalid.
        // We'll just exit and let the UI handle it gracefully (if needed).
        LOG_WARNING("Failed to get current user: " << ex.what() << std::endl);
        return std::nullopt;
    }

    try {
        const auto uuidFileName = shared::generateUuidv4();
        auto speedCheckDestination = service::api::getGlobalApi()->getSpeedCheckUri(uuidFileName);
        auto pipe = std::make_unique<service::recorder::pipe::Pipe>(uuidFileName);
        auto piper = std::make_unique<service::recorder::pipe::CloudStoragePiper>(uuidFileName, speedCheckDestination, std::move(pipe), false);
        service::recorder::pipe::PipeClient pipeClient(uuidFileName);

        // 160MB buffer. writeBuffer is divisible by the cloud_storage_piper's CLOUD_BUFFER_SIZE_BYTES. 
        std::vector<char> writeBuffer(1024 * 1024 * 160, 'A');
        int lastUl = 0;

        // This continues running after the piper and pipeClient stop. LastUl continues changing
        piper->setCurlProgressCallback([&lastUl](size_t dltotal, size_t dl, size_t ultotal, size_t ul){
            lastUl = ul;
            if(ul == ultotal) {
                lastUl = 0;
            }
        });
        
        std::thread t1([&piper]() {
            piper->start();
            // Listen for 10 seconds
            std::this_thread::sleep_for(std::chrono::seconds(10));
            // stopAndSkipFlush() is only called for speed_Check, as we don't need to send up the final bytes 
            piper->stopAndSkipFlush();
        });
        pipeClient.start(writeBuffer);
        if (t1.joinable()) {
            t1.join();
        }
        pipeClient.stop();

        // this gives confidence that lastUl won't change underneath us.
        double finalUl = lastUl;
        piper->wait(); 

        // This should calculate out the Mb/s
        double millisecondsSpentUploading = piper->getMillisecondsSpentUploading().count();
        double secondsSpentUploading = millisecondsSpentUploading/1000;
        double speedCheckRes = (piper->getUploadedBytes() + finalUl) * 8 / 1024 / 1024;

        // finalUl lets us know if there was any partial uploads to be included.
        report.standardUploadMbps = speedCheckRes / ((finalUl == 0) ? secondsSpentUploading : 10);
        service::api::getGlobalApi()->postSpeedCheck(report.standardUploadMbps, uuidFileName);
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to do speed check: " << ex.what() << std::endl);
        return std::nullopt;
    }

    return report;
}

std::optional<WowSetupReport> generateWowReport(shared::EGame game) {
    WowSetupReport report;

    LOG_INFO("Generating Report for: " << shared::gameToString(game) << std::endl);
    const auto wowPath = shared::system::getExecutableInstalledPath(game);
    if (!wowPath) {
        return std::nullopt;
    }
    LOG_INFO("...Installed at: " << wowPath.value() << std::endl);

    const auto wowRelease = shared::gameToWowRelease(game);
    const auto installedAddons = shared::system::hasWowCombatLogAddonInstalled(wowPath.value(), wowRelease);
    
    report.hasClAddon = !installedAddons.empty();
    report.hasMultipleAddons = installedAddons.size() > 1;
    
    for (const auto& a: installedAddons) {
        report.hasRecommendedAddon |= shared::system::isRecommendedCombatLogAddon(a, wowRelease);
    }

    report.addons = installedAddons;

    if (!report.hasClAddon) {
        LOG_INFO("\t...No Combat Log Addon Installed - Installing." << std::endl);
        shared::system::installCombatLogAddonForWow(wowPath.value(), wowRelease);
        report.installedAddon = true;
    }

    if (!shared::system::hasWowAdvancedCombatLoggingTurnedOn(wowPath.value())) {
        LOG_INFO("\t...Turning on advanced combat log." << std::endl);
        shared::system::turnOnAdvancedCombatLoggingForWow(wowPath.value());
        report.turnedOnAdvanced = true;
    }
    return report;
}

void touchFile(const fs::path& pth) {
    std::ofstream output(pth);
    output.close();
}

int main(int argc, char** argv) {
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    shared::log::Log::initializeGlobalLogger("automated_game_setup.log");

    LOG_INFO("Starting Automated Game Setup..." << std::endl);
    service::system::getCurrentSettings()->reloadSettingsFromFile();

    po::options_description desc("Options");
    desc.add_options()
        ("output", po::value<std::string>()->required(), "Output report location.");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    SetupReport report;
    const auto reportCheckFile = shared::filesystem::getSquadOvUserFolder() / fs::path(".agreport");
    report.firstTime = !fs::exists(reportCheckFile);
    touchFile(reportCheckFile);

    // For the speed-check, we want to do it if it hasn't been done before. The old system
    // was to store this in the settings file but we want to transition that to the new system
    // of just singular files in the user app data directory.
    const auto speedCheckFile = shared::filesystem::getSquadOvUserFolder() / fs::path(".speedcheck");
    bool ranSpeedCheck = fs::exists(speedCheckFile);
    auto rawData = service::system::getCurrentSettings()->raw();
    if (rawData.find("ranSpeedCheck") != rawData.end() && rawData.find("speedCheckResultMbps") != rawData.end()) {
        ranSpeedCheck |= rawData.value("ranSpeedCheck", false);
        if (ranSpeedCheck) {
            touchFile(speedCheckFile);
        }
    }

    if (!ranSpeedCheck) {
        report.speedCheck = generateSpeedCheckReport();
        if (report.speedCheck) {
            report.needsAttention = true;
            touchFile(speedCheckFile);
        }
    }

    const auto wowRetailFile = shared::filesystem::getSquadOvUserFolder() / fs::path(".wowretail");
    if (!fs::exists(wowRetailFile)) {
        report.wowRetail = generateWowReport(shared::EGame::WoW);
        if (report.wowRetail) {
            touchFile(wowRetailFile);
            const auto fail = (!report.wowRetail.value().hasRecommendedAddon || report.wowRetail.value().installedAddon);
            report.needsAttention |= fail;
            if (!fail) {
                report.wowRetail = std::nullopt;
            }
        }
    }

    const auto wowTbcFile = shared::filesystem::getSquadOvUserFolder() / fs::path(".wowtbc");
    if (!fs::exists(wowTbcFile)) {
        report.wowTbc = generateWowReport(shared::EGame::WowTbc);
        if (report.wowTbc) {
            touchFile(wowTbcFile);

            const auto fail = (!report.wowTbc.value().hasRecommendedAddon || report.wowTbc.value().installedAddon);
            report.needsAttention |= fail;
            if (!fail) {
                report.wowTbc = std::nullopt;
            }
        }
    }

    const auto wowClassicFile = shared::filesystem::getSquadOvUserFolder() / fs::path(".wowclassic");
    if (!fs::exists(wowClassicFile)) {
        report.wowClassic = generateWowReport(shared::EGame::WowVanilla);
        if (report.wowClassic) {
            touchFile(wowClassicFile);

            const auto fail = (!report.wowClassic.value().hasRecommendedAddon || report.wowClassic.value().installedAddon);
            report.needsAttention |= fail;
            if (!fail) {
                report.wowClassic = std::nullopt;
            }
        }
    }

    const auto hwCheckFile = shared::filesystem::getSquadOvUserFolder() / fs::path(".hwpref");
    if (!fs::exists(hwCheckFile)) {
        report.needsHwCheck = true;
        report.needsAttention = true;
        touchFile(hwCheckFile);
    }

    const auto reportFname = fs::path(vm["output"].as<std::string>());
    LOG_INFO("Generating report to: " << reportFname << std::endl);
    std::ofstream ofs(reportFname);
    ofs << std::setw(4) << shared::json::JsonConverter<SetupReport>::to(report);
    ofs.close();
    return 0;
}