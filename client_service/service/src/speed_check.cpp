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
    // const service::recorder::pipe::PipePtr& rawPipe = piper->getPipePtr();
    std::thread t1([&piper]() {
        piper->start();
        std::this_thread::sleep_for(std::chrono::milliseconds(10000)); // Listen for 15 seconds
        piper->pauseProcessingFromPipe(true);
    });
    std::vector<char> writeBuffer(1024 * 1024 * 100, 'A');
    pipeClient.start(writeBuffer);
    // std::thread t2([&pipeClient]() {
    //     std::vector<char> writeBuffer(1024 * 1024 * 10, 'A');
    //     pipeClient.start(writeBuffer);
    //     std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    //     pipeClient.stop();
    // });
    if (t1.joinable())
    {
        t1.join();
    }

    pipeClient.stop();
    // if (t2.joinable())
    // {
        // t2.join();
    // }

    speed_check_res = piper->getUploadedBytes();
    LOG_INFO("Check getUploadedBytes: " << speed_check_res << std::endl); // TODO-FIX: This gets called before the final transfer that has a few last bytes
    // Ideally, it should get the last bytes as well, and then get the UploadedBytes

    shared::squadov::SpeedCheckData speedCheckData;
    speedCheckData.speedMbs = speed_check_res;
    // TODO: Update SQL database for user with the speedtest results.
    // Alternatively: Pass it up to the Client, and do a user call. Either works.

    // service::api::getGlobalApi()->postSpeedCheck(speedCheckData, uuidFileName);

    // service::api::getGlobalApi()->deleteSpeedCheckFile(uuidFileName);

    return 0;
}