#include "process_watcher/watcher.h"
#include "shared/games.h"
#include "shared/filesystem/common_paths.h"
#include "aimlab/aimlab_process_handler.h"
#include "valorant/valorant_process_handler.h"
#include "database/api.h"
#include "zeromq/zeromq.h"
#include "shared/errors/error.h"
#include "shared/log/log.h"

#include <boost/stacktrace.hpp>
#include <chrono>
#include <cstdio>
#include <curl/curl.h>
#include <filesystem>
#include <iostream>
#include <portaudio.h>
#include <thread>

extern "C" {
#include <libavutil/log.h>
}

#ifdef _WIN32
#include <Windows.h>
#endif

namespace fs = std::filesystem;

process_watcher::ProcessWatcher watcher;

#ifdef _WIN32

LONG handleTopLevelExceptions(EXCEPTION_POINTERS* ex) {
    LOG_ERROR(
        "SquadOV Fatal Error [Code: " << ex->ExceptionRecord->ExceptionCode << "]" << std::endl
            << boost::stacktrace::stacktrace() << std::endl
    );
    return EXCEPTION_CONTINUE_SEARCH;
}

#endif

void ffmpegLogCallback(void* ptr, int level, const char* fmt, va_list v1) {
    if (level > av_log_get_level()) {
        return;
    }
    vprintf(fmt, v1);
    fflush(stdout);
}

int main(int argc, char** argv) {

#ifdef _WIN32
    SetUnhandledExceptionFilter(handleTopLevelExceptions);
#endif

    if (curl_global_init(CURL_GLOBAL_ALL)) {
        THROW_ERROR("Failed to initialize CURL.");
    }

    if (Pa_Initialize() != paNoError) {
        THROW_ERROR("Failed to initialize PortAudio.");
    }

    // Start running the ZeroMQ server for IPC communication with the frontend.
    service::zeromq::ZeroMQServerClient zeroMqServerClient;
    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_SESSION_ID_TOPIC, [](const std::string& msg){
        LOG_INFO("RECEIVED NEW SESSION ID: " << msg << std::endl);
    });
    zeroMqServerClient.start();

    // Init FFmpeg logging - not sure why the default ffmpeg logging isn't working.
    av_log_set_callback(ffmpegLogCallback);

    // Connect to the database which should be in the predefined spot in the APPDATA (or its OS equivalent) folder.
    const fs::path dbFname = shared::filesystem::getSquadOvUserFolder() / fs::path("squadov.db");
    const auto db = std::make_unique<service::database::DatabaseApi>(dbFname.string());

    // Start process watcher to watch for our supported games.
    watcher.beginWatchingGame(shared::EGame::Valorant, std::make_unique<service::valorant::ValorantProcessHandler>(db.get()));
    watcher.beginWatchingGame(shared::EGame::Aimlab, std::make_unique<service::aimlab::AimlabProcessHandler>(db.get()));
    watcher.start();

    curl_global_cleanup();
    Pa_Terminate();
}