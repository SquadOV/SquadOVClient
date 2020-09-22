#include "process_watcher/watcher.h"
#include "shared/games.h"
#include "shared/filesystem/common_paths.h"
#include "valorant/valorant_process_handler.h"
#include "database/api.h"

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

namespace fs = std::filesystem;

process_watcher::ProcessWatcher watcher;

void ffmpeg_log_callback(void* ptr, int level, const char* fmt, va_list v1) {
    if (level > av_log_get_level()) {
        return;
    }
    vprintf(fmt, v1);
    fflush(stdout);
}


int main(int argc, char** argv) {
    if (curl_global_init(CURL_GLOBAL_ALL)) {
        throw std::runtime_error("Failed to initialize CURL.");
    }

    if (Pa_Initialize() != paNoError) {
        throw std::runtime_error("Failed to initialize PortAudio.");
    }

    // Init FFmpeg logging - not sure why the default ffmpeg logging isn't working.
    av_log_set_callback(ffmpeg_log_callback);

    // Connect to the database which should be in the predefined spot in the APPDATA (or its OS equivalent) folder.
    const fs::path dbFname = shared::filesystem::getSquadOvFolder() / fs::path("squadov.db");
    const auto db = std::make_unique<service::database::DatabaseApi>(dbFname.string());

    // Start process watcher to watch for our supported games.
    watcher.beginWatchingGame(shared::EGame::Valorant, std::make_unique<service::valorant::ValorantProcessHandler>(db.get()));
    watcher.start();

    curl_global_cleanup();
    Pa_Terminate();
}