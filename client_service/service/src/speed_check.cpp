#include "shared/log/log.h"
#include "api/squadov_api.h"
#include "recorder/pipe/cloud_storage_piper.h"
#include "recorder/pipe/pipe_client.h"
#include "recorder/pipe/pipe.h"
#include "shared/http/http_client.h"
#include "shared/uuid.h"
#include "system/settings.h"
#include "shared/time/ntp_client.h"
#include "uploader/uploader.h"
#include "shared/filesystem/utility.h"
#include "shared/filesystem/common_paths.h"
#include "shared/env.h"

#include <chrono>
#include <curl/curl.h>
#include <exception>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace fs = std::filesystem;

int main(int argc, char **argv) {
    shared::log::Log::initializeGlobalLogger("squadov_speed_check.log");
    // NTP can't be init before the logger since we log stuff inside the NTP client.
    shared::time::NTPClient::singleton()->enable(true);
    shared::time::NTPClient::singleton()->initialize();
    LOG_INFO("EXE PATH: " << shared::filesystem::getCurrentExeFolder() << std::endl);
    LOG_INFO("Retrieve Session ID from ENV" << std::endl);
    try {
        // Note that setSessionId also does an API call to pull the current user.
        service::api::getGlobalApi()->setSessionId(shared::getEnv("SQUADOV_SESSION_ID", ""));
    } catch (std::exception& ex) {
        // This needs to be caught because the session could be completely invalid.
        // We'll just exit and let the UI handle it gracefully (if needed).
        LOG_WARNING("Failed to get current user: " << ex.what() << std::endl);
        std::exit(1);
    }
    const auto uuidFileName = shared::generateUuidv4();
    auto speedCheckDestination = service::api::getGlobalApi()->getSpeedCheckUri(uuidFileName);
    auto pipe = std::make_unique<service::recorder::pipe::Pipe>(uuidFileName);
    auto piper = std::make_unique<service::recorder::pipe::CloudStoragePiper>(uuidFileName, speedCheckDestination, std::move(pipe));
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
    double speedCheckResMbps = speedCheckRes / ((finalUl == 0) ? secondsSpentUploading : 10);
    service::api::getGlobalApi()->postSpeedCheck(speedCheckResMbps, uuidFileName);

    // We use this data in the settings file to recommend if the user should use automatic upload.
    LOG_INFO("Saving Speed Check results to disk..." << std::endl);
    auto rawData = service::system::getCurrentSettings()->raw();
    rawData["ranSpeedCheck"] = shared::json::JsonConverter<bool>::to(true);
    rawData["speedCheckResultMbps"] = shared::json::JsonConverter<int>::to(speedCheckResMbps);

    // If the user is uploading slower than 8 Mb/s, disable Automatic Upload.
    if(speedCheckResMbps < 8) {
        rawData["record"]["useLocalRecording"] = shared::json::JsonConverter<bool>::to(true);
    }
    const auto fpath = shared::filesystem::getSquadOvUserSettingsFile();
    const auto tmpPath = fpath.parent_path() / fs::path("settings.json.tmp");
    std::ofstream ofs(tmpPath);
    ofs << std::setw(4) << rawData;
    ofs.close();
    fs::rename(tmpPath, fpath);

    // Calling an exit because the code will have done everything it needs to do at this point.
    // If the user has really slow internet, they would have to wait until it fully sends the 16MB which is unnecessary at this point
    std::exit(0);
    return 0;
}