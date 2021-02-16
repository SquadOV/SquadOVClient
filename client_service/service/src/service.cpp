#include "process_watcher/watcher.h"
#include "shared/games.h"
#include "shared/filesystem/common_paths.h"
#include "aimlab/aimlab_process_handler.h"
#include "valorant/valorant_process_handler.h"
#include "hearthstone/hearthstone_process_handler.h"
#include "wow/wow_process_handler.h"
#include "league/league_process_handler.h"
#include "system/state.h"
#include "zeromq/zeromq.h"
#include "shared/env.h"
#include "shared/errors/error.h"
#include "shared/log/log.h"
#include "api/squadov_api.h"
#include "api/kafka_api.h"
#include "game_event_watcher/hearthstone/hearthstone_log_watcher.h"
#include "vod/vod_clipper.h"
#include "recorder/audio/portaudio_audio_recorder.h"

#include <boost/program_options.hpp>
#include <boost/stacktrace.hpp>
#include <chrono>
#include <cstdio>
#include <curl/curl.h>
#include <date/tz.h>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <portaudio.h>
#include <thread>

#include "system/win32/hwnd_utils.h"

extern "C" {
#include <libavutil/log.h>
}

#ifdef _WIN32
#include <Windows.h>
#endif

namespace fs = std::filesystem;
namespace po = boost::program_options;

process_watcher::ProcessWatcher watcher;

#ifdef _WIN32

LONG handleTopLevelExceptions(EXCEPTION_POINTERS* ex) {
    const auto dumpLoc = shared::log::generateMinidump(ex);
    LOG_ERROR(
        "SquadOV Fatal Error [Code: " << ex->ExceptionRecord->ExceptionCode << "] @ " << dumpLoc << std::endl
            << boost::stacktrace::stacktrace() << std::endl
    );
    return EXCEPTION_CONTINUE_SEARCH;
}

#endif

void ffmpegLogCallback(void* ptr, int level, const char* fmt, va_list v1) {
    if (level > av_log_get_level()) {
        return;
    }

    char buffer[2048];
    vsprintf(buffer, fmt, v1);

    std::string sBuffer(buffer);
    LOG_INFO(sBuffer);
}

void defaultMain() {
    const auto flags = service::api::getGlobalApi()->getSessionFeatures();

    // Start process watcher to watch for our supported games.
    watcher.beginWatchingGame(shared::EGame::Valorant, std::make_unique<service::valorant::ValorantProcessHandler>());
    watcher.beginWatchingGame(shared::EGame::Aimlab, std::make_unique<service::aimlab::AimlabProcessHandler>());
    watcher.beginWatchingGame(shared::EGame::Hearthstone, std::make_unique<service::hearthstone::HearthstoneProcessHandler>());
    watcher.beginWatchingGame(shared::EGame::WoW, std::make_unique<service::wow::WoWProcessHandler>());
    // Note that this covers both League of Legends and Teamfight Tactics as they both share the same game executable.
    if (flags.enableTft || flags.enableLol) {
        watcher.beginWatchingGame(shared::EGame::LeagueOfLegends, std::make_unique<service::league::LeagueProcessHandler>());
    }
    watcher.start();
}

// Not too big a fan of making this a gigantic executable that does everything..hmm..
void wowTest(const std::string& log, const std::string& vod, const std::string& vodTime) {
    service::wow::WoWProcessHandler handler;
    handler.manualStartLogWatching(std::filesystem::path(log), std::filesystem::path(vod), shared::strToTime(vodTime));
}

int main(int argc, char** argv) {
    po::options_description desc("Options");
    desc.add_options()
        ("mode", po::value<std::string>()->default_value(""), "Client mode.")
        ("log", po::value<std::string>()->default_value(""), "Log file to manually parse (mode: wow_test).")
        ("vod", po::value<std::string>()->default_value(""), "VOD file to manually upload (mode: wow_test).")
        ("vodTime", po::value<std::string>()->default_value(""), "The UTC timestamp of when the VOD started to record (mode: wow_test).");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    if (shared::getEnv("SQUADOV_DEBUG") != "") {
        LOG_INFO("Enabling Debug Logs" << std::endl);
        shared::log::getGlobalLogger().setThreshold(shared::log::LogType::Debug);   
    }

    LOG_INFO("Start SquadOV" << std::endl);
#ifdef _WIN32
    SetUnhandledExceptionFilter(handleTopLevelExceptions);
#endif

    if (curl_global_init(CURL_GLOBAL_ALL)) {
        THROW_ERROR("Failed to initialize CURL.");
    }

    if (Pa_Initialize() != paNoError) {
        THROW_ERROR("Failed to initialize PortAudio.");
    }

    const auto tzDataFolder = shared::filesystem::getSquadOvTzDataFolder();
    LOG_INFO("TZ FOLDER: " << tzDataFolder << std::endl);
    date::set_install(tzDataFolder);
    date::get_tzdb_list();

    LOG_INFO("USER FOLDER: " << shared::filesystem::getSquadOvUserFolder() << std::endl);

    // Start running the ZeroMQ server for IPC communication with the frontend.
    service::zeromq::ZeroMQServerClient zeroMqServerClient;
    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_SESSION_ID_TOPIC, [](const std::string& msg){
        LOG_INFO("RECEIVE SESSION ID: " << msg << std::endl);
        service::api::getGlobalApi()->setSessionId(msg);
    });
    zeroMqServerClient.start();

    service::api::getGlobalApi()->setSessionIdUpdateCallback([&zeroMqServerClient](const std::string& sessionId){
        LOG_INFO("SEND SESSION ID: " << sessionId << std::endl);
        zeroMqServerClient.sendMessage(service::zeromq::ZEROMQ_SESSION_ID_TOPIC, sessionId);
        service::api::getGlobalApi()->setSessionId(sessionId);
    });

    LOG_INFO("Retrieve Session ID from ENV" << std::endl);
    try {
        // Note that setSessionId also does an API call to pull the current user.
        service::api::getGlobalApi()->setSessionId(shared::getEnv("SQUADOV_SESSION_ID", ""));
    } catch (std::exception& ex) {
        // This needs to be caught because the session could be completely invalid.
        // We'll just exit and let the UI handle it gracefully (if needed).
        LOG_WARNING("Failed to get current user: " << ex.what() << std::endl);

        // This is needed to hand back control to the UI.
        // Maybe use a specific failure message?
        zeroMqServerClient.sendMessage(service::zeromq::ZEROMQ_READY_TOPIC, "");
        std::exit(1);
    }

    service::api::getGlobalApi()->retrieveSessionFeatureFlags();

    LOG_INFO("Initialize Kafka API" << std::endl);
    service::api::getKafkaApi()->initialize();

    // Initialize global state and start to listen to messages coming via ZeroMQ about
    // how to update certain state.
    service::system::getGlobalState();

    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_CHANGE_PAUSE_TOPIC, [](const std::string& msg) {
        const auto json = nlohmann::json::parse(msg);
        const auto paused = json.get<bool>();
        LOG_INFO("User Change Pause: " << paused << std::endl);
        service::system::getGlobalState()->setPause(paused);
    });

    LOG_INFO("Registering ZeroMQ Audio Device Callbacks" << std::endl);
    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_REQUEST_AUDIO_INPUT_TOPIC, [&zeroMqServerClient](const std::string&) {
        zeroMqServerClient.sendMessage(
            service::zeromq::ZEROMQ_RESPOND_AUDIO_INPUT_TOPIC,
            service::recorder::audio::PortaudioAudioRecorder::getDeviceListing(service::recorder::audio::EAudioDeviceDirection::Input).toJson().dump()
        );
    });

    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_REQUEST_AUDIO_OUTPUT_TOPIC, [&zeroMqServerClient](const std::string&) {
        zeroMqServerClient.sendMessage(
            service::zeromq::ZEROMQ_RESPOND_AUDIO_OUTPUT_TOPIC,
            service::recorder::audio::PortaudioAudioRecorder::getDeviceListing(service::recorder::audio::EAudioDeviceDirection::Output).toJson().dump()
        );
    });

    LOG_INFO("Send Ready" << std::endl);
    // At this point we can fire off an event letting the UI know that the service is ready.
    // The reason we need this is because setSessionId will fire off an API request to get the
    // user's profile. The same call also happens on the UI side to validate the session ID once
    // the UI starts. However, in the case the session ID is expired then one of these two requests
    // will have an invalid session ID (the UI one since the backend service is started first) as
    // the session ID update won't have propagated yet. So the only choice we have is to let the
    // backend service run first and then only then allow the UI to proceed.
    zeroMqServerClient.sendMessage(service::zeromq::ZEROMQ_READY_TOPIC, "");

    // Init FFmpeg logging - not sure why the default ffmpeg logging isn't working.
    LOG_INFO("Init FFMpeg Logging" << std::endl);
    av_log_set_level(AV_LOG_VERBOSE);
    av_log_set_callback(ffmpegLogCallback);

    // Some games need some other setup if they're installed. Do this every time the app starts up as
    // doing it when the game is already running would be too late.
    LOG_INFO("Enable Hearthstone Logging" << std::endl);
    game_event_watcher::HearthstoneLogWatcher::enableHearthstoneLogging();

    LOG_INFO("Registering State callbacks" << std::endl);
    service::system::getGlobalState()->addGameRunningCallback([&zeroMqServerClient](const shared::EGameSet& set){
        const auto setVec = shared::gameSetToVector(set);
        zeroMqServerClient.sendMessage(
            service::zeromq::ZEROMQ_RUNNING_GAMES_TOPIC,
            shared::gameVectorToJsonArray(setVec).dump()
        );
    });

    service::system::getGlobalState()->addGameRecordingCallback([&zeroMqServerClient](const shared::EGameSet& set){
        const auto setVec = shared::gameSetToVector(set);
        zeroMqServerClient.sendMessage(
            service::zeromq::ZEROMQ_RECORDING_GAMES_TOPIC,
            shared::gameVectorToJsonArray(setVec).dump()
        );
    });

    LOG_INFO("Add VOD Clip request handler..." << std::endl);    
    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_REQUEST_VOD_CLIP_TOPIC, [&zeroMqServerClient](const std::string& msg){
        LOG_INFO("RECEIVE VOD CLIP REQUEST: " << msg << std::endl);
        std::thread t([&zeroMqServerClient, msg](){
            const auto json = nlohmann::json::parse(msg);
            const auto resp = service::vod::vodClip(service::vod::VodClipRequest::fromJson(json));
            zeroMqServerClient.sendMessage(
                service::zeromq::ZEROMQ_RESPOND_VOD_CLIP_TOPIC,
                resp.toJson().dump()
            );
        });
        t.detach();
    });
    
    const auto mode = vm["mode"].as<std::string>();
    if (mode == "") {
        defaultMain();
    } else if (mode == "wow_test") {
        wowTest(vm["log"].as<std::string>(), vm["vod"].as<std::string>(), vm["vodTime"].as<std::string>());
    } else {
        THROW_ERROR("Unknown Mode: " << mode);
    }

    curl_global_cleanup();
    Pa_Terminate();
    return 0;
}