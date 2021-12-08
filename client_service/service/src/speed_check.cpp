#include "shared/log/log.h"
#include "api/squadov_api.h"
#include "recorder/pipe/cloud_storage_piper.h"
#include "recorder/pipe/pipe_client.h"
#include "recorder/pipe/pipe.h"
#include "shared/http/http_client.h"
#include "shared/uuid.cpp"
#include "shared/time/ntp_client.h"
#include "shared/squadov/speed_check.h"
#include "uploader/uploader.h"
#include "shared/filesystem/common_paths.h"
#include "shared/filesystem/utility.h"
#include "shared/filesystem/local_record.h"
#include "shared/env.h"

#include <chrono>
#include <codecvt>
#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <date/tz.h>
#include <exception>
#include <nlohmann/json.hpp>
#include <thread>

#include "shared/system/win32/hwnd_utils.h"

extern "C"
{
#include <libavutil/log.h>
}

#ifdef _WIN32
#include <Windows.h>
#endif

int main(int argc, char **argv)
{
    shared::log::Log::initializeGlobalLogger("squadov.log");

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
    size_t speed_check_res;
    service::recorder::pipe::PipeClient pipeClient(uuidFileName);

    // setSkipLastCall() is only called for speed_Check, as we don't need to send up the final bytes 
    piper->setSkipLastCall(true);
    std::thread t1([&piper]() {
        piper->start();
        std::this_thread::sleep_for(std::chrono::seconds(10)); // Listen for 10 seconds
        piper->stopAndSkipFlush(); 

    });
    std::vector<char> writeBuffer(1024 * 1024 * 200, 'A');
    pipeClient.start(writeBuffer);
    if (t1.joinable())
    {
        t1.join();
    }
    pipeClient.stop();
    piper->wait(); 
    speed_check_res = piper->getUploadedBytes();

    // This should calculate out the MB/s
    auto speed_check_res_mbps = speed_check_res/1024/1024/10;

    shared::squadov::SpeedCheckData speedCheckData;
    speedCheckData.speed_mbps = speed_check_res_mbps;

    service::api::getGlobalApi()->postSpeedCheck(speedCheckData, uuidFileName);

    return 0;
}