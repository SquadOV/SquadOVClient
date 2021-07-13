#include "shared/time/ntp_client.h"
#include "process_watcher/watcher.h"
#include "shared/games.h"
#include "shared/filesystem/common_paths.h"
#include "shared/filesystem/utility.h"
#include "shared/filesystem/local_record.h"
#include "aimlab/aimlab_process_handler.h"
#include "csgo/csgo_process_handler.h"
#include "valorant/valorant_process_handler.h"
#include "hearthstone/hearthstone_process_handler.h"
#include "wow/wow_process_handler.h"
#include "league/league_process_handler.h"
#include "system/state.h"
#include "system/ipc.h"
#include "zeromq/zeromq.h"
#include "shared/env.h"
#include "shared/errors/error.h"
#include "shared/log/log.h"
#include "shared/strings/strings.h"
#include "api/squadov_api.h"
#include "api/kafka_api.h"
#include "game_event_watcher/hearthstone/hearthstone_log_watcher.h"
#include "game_event_watcher/csgo/csgo_log_watcher.h"
#include "game_event_watcher/csgo/csgo_gsi_listener.h"
#include "vod/vod_clipper.h"
#include "recorder/game_preview_stream.h"
#include "recorder/audio/portaudio_audio_recorder.h"
#include "recorder/audio/win32/wasapi_interface.h"
#include "recorder/pipe/cloud_storage_piper.h"
#include "system/settings.h"
#include "system/win32/message_loop.h"
#include "api/local_api.h"

#include <boost/program_options.hpp>
#include <boost/stacktrace.hpp>
#include <civetweb.h>
#include <chrono>
#include <codecvt>
#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <date/tz.h>
#include <exception>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <portaudio.h>
#include <thread>

#include "shared/system/win32/hwnd_utils.h"

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
    LOG_ERROR("Executing unhandled exception hanlder..." << std::endl);
    const auto dumpLoc = shared::log::generateMinidump(ex);
    LOG_ERROR(
        "SquadOV Fatal Error [Code: " << ex->ExceptionRecord->ExceptionCode << "] @" << dumpLoc << std::endl
            << boost::stacktrace::stacktrace() << std::endl
    );
    LOG_FLUSH();
    return EXCEPTION_CONTINUE_SEARCH;
}

#endif

void onSquadovExit() {
    LOG_INFO("SquadOV Exiting...Good Bye!" << std::endl);
    LOG_FLUSH();
}

void onSquadovTerminate() {
    LOG_ERROR("SquadOV Termination Detected" << std::endl);
    LOG_FLUSH();
}

void ffmpegLogCallback(void* ptr, int level, const char* fmt, va_list v1) {
    if (level > av_log_get_level()) {
        return;
    }

    char buffer[2048];
    vsprintf(buffer, fmt, v1);

    std::string sBuffer(buffer);
    LOG_INFO(sBuffer);
}

bool enablePaDebugLogs = false;
extern "C" {
typedef void (*PaUtilLogCallback ) (const char *log);
extern void PaUtil_SetDebugPrintFunction(PaUtilLogCallback  cb);
}

void portaudioLogCallback(const char* log) {
    if (enablePaDebugLogs) {
        LOG_INFO("PORTAUDIO: " << log);
    }
}

void defaultMain() {
    const auto flags = service::api::getGlobalApi()->getSessionFeatures();

    // Start process watcher to watch for our supported games.
    watcher.beginWatchingGame(shared::EGame::Valorant, std::make_unique<service::valorant::ValorantProcessHandler>());
    watcher.beginWatchingGame(shared::EGame::Aimlab, std::make_unique<service::aimlab::AimlabProcessHandler>());
    watcher.beginWatchingGame(shared::EGame::Hearthstone, std::make_unique<service::hearthstone::HearthstoneProcessHandler>());
    watcher.beginWatchingGame(shared::EGame::WoW, std::make_unique<service::wow::WoWProcessHandler>());
    // Note that this covers both League of Legends and Teamfight Tactics as they both share the same game executable.
    watcher.beginWatchingGame(shared::EGame::LeagueOfLegends, std::make_unique<service::league::LeagueProcessHandler>());
    watcher.beginWatchingGame(shared::EGame::CSGO, std::make_unique<service::csgo::CsgoProcessHandler>());
    watcher.start();
}

// Not too big a fan of making this a gigantic executable that does everything..hmm..
void wowTest(const std::string& log, const std::string& vod, const std::string& vodTime) {
    service::wow::WoWProcessHandler handler;
    handler.manualStartLogWatching(std::filesystem::path(log), std::filesystem::path(vod), shared::strToTime(vodTime));
}

int main(int argc, char** argv) {
#ifdef _WIN32
    // I think this is needed because we aren't generally calling startRecording on the same thread as Pa_Initialize?
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif
    shared::log::Log::initializeGlobalLogger("squadov.log");

    // NTP can't be init before the logger since we log stuff inside the NTP client.
    shared::time::NTPClient::singleton()->enable(service::system::getCurrentSettings()->enableNtp());
    shared::time::NTPClient::singleton()->initialize();

    LOG_INFO("EXE PATH: " << shared::filesystem::getCurrentExeFolder() << std::endl);

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
        shared::log::Log::singleton()->setThreshold(shared::log::LogType::Debug);
        enablePaDebugLogs = true;
    }

    LOG_INFO("Start SquadOV" << std::endl);
#ifdef _WIN32
    LOG_INFO("Set unhandled exception filter..." << std::endl);
    SetUnhandledExceptionFilter(handleTopLevelExceptions);
#endif
    LOG_INFO("Set atexit..." << std::endl);
    std::atexit(onSquadovExit);
    LOG_INFO("Set atquickexit..." << std::endl);
    std::at_quick_exit(onSquadovExit);
    LOG_INFO("Set terminate..." << std::endl);
    std::set_terminate(onSquadovTerminate);
    
    LOG_INFO("Initialize CURL..." << std::endl);
    if (curl_global_init(CURL_GLOBAL_ALL)) {
        THROW_ERROR("Failed to initialize CURL.");
    }

    LOG_INFO("Port Audio Set Debug Logging..." << std::endl);
    PaUtil_SetDebugPrintFunction(portaudioLogCallback);

    const auto tzDataFolder = shared::filesystem::getSquadOvTzDataFolder();
    LOG_INFO("TZ FOLDER: " << tzDataFolder << std::endl);
    date::set_install(tzDataFolder);
    date::get_tzdb_list();

    // Sanity check the timezone in the logs...
    {
        const auto gmt = shared::nowUtc();
        const auto local = date::make_zoned(date::current_zone(), gmt);
        const auto checkGmt = date::make_zoned(
            date::current_zone(),
            shared::strToLocalTime(
                shared::localTimeToString(local.get_local_time()),
                "%Y-%m-%d %T"
            )
        ).get_sys_time();
        LOG_INFO("Current TimeZone: " << date::current_zone()->name() << std::endl);
        LOG_INFO("\tGMT: " << shared::timeToStr(gmt) << std::endl);
        LOG_INFO("\tLocal: " << shared::localTimeToString(local.get_local_time()) << std::endl);
        LOG_INFO("\tCheck GMT: " << shared::timeToStr(checkGmt) << std::endl);
    }

    LOG_INFO("Set Locale..." << std::endl);
    std::setlocale(LC_ALL, "en_US.utf8");

    LOG_INFO("Init CivetWeb..." << std::endl);
    mg_init_library(0);

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

    LOG_INFO("Cleaning up temporary files..." << std::endl);
    fs::remove_all(shared::filesystem::getSquadOvTempFolder());
    fs::create_directories(shared::filesystem::getSquadOvTempFolder());

    fs::remove_all(shared::filesystem::getSquadOvDvrSessionFolder());
    fs::create_directories(shared::filesystem::getSquadOvDvrSessionFolder());

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
            service::recorder::audio::win32::WASAPIInterface::getDeviceListing(service::recorder::audio::EAudioDeviceDirection::Input).toJson().dump()
        );
    });

    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_REQUEST_AUDIO_OUTPUT_TOPIC, [&zeroMqServerClient](const std::string&) {
        zeroMqServerClient.sendMessage(
            service::zeromq::ZEROMQ_RESPOND_AUDIO_OUTPUT_TOPIC,
            service::recorder::audio::win32::WASAPIInterface::getDeviceListing(service::recorder::audio::EAudioDeviceDirection::Output).toJson().dump()
        );
    });

    LOG_INFO("Starting Local API server..." << std::endl);
    service::api::LocalApiServer apiServer;
    apiServer.start();

    zeroMqServerClient.sendMessage(service::zeromq::ZEROMQ_LOCAL_API_PORT_TOPIC, std::to_string(apiServer.port()));

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

    // Some games need some other setup if they're installed. Do this every time the app starts up because
    // doing it when the game is already running would be too late.
    LOG_INFO("Enable Hearthstone Logging" << std::endl);
    game_event_watcher::HearthstoneLogWatcher::enableHearthstoneLogging();

    LOG_INFO("Enable CS:GO Logging/GSI" << std::endl);
    game_event_watcher::CsgoLogWatcher::enableCsgoLogging();
    game_event_watcher::CsgoGsiListener::enableCsgoGsi();

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

    LOG_INFO("Add Clip upload handler..." << std::endl);
    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_REQUEST_GCS_UPLOAD_TOPIC, [&zeroMqServerClient](const std::string& msg){
        LOG_INFO("RECEIVE GCS UPLOAD REQUEST: " << msg << std::endl);
        /*
        std::thread t([&zeroMqServerClient, msg](){
            const auto json = nlohmann::json::parse(msg);
            const auto request = service::recorder::pipe::GCSUploadRequest::fromJson(json);

            shared::TimePoint lastProgressTm = shared::nowUtc();
            const auto resp = service::recorder::pipe::uploadToGcs(request, [&zeroMqServerClient, request, &lastProgressTm](size_t dl, size_t total){
                const auto now = shared::nowUtc();
                const auto threshold = lastProgressTm + std::chrono::seconds(1);
                if (now > threshold) {
                    zeroMqServerClient.sendMessage(
                        service::zeromq::ZEROMQ_GCS_UPLOAD_PROGRESS_TOPIC,
                        nlohmann::json{
                            {"task", request.task},
                            {"download", dl},
                            {"total", total},
                            {"done", false}
                        }.dump(),
                        true
                    );
                    lastProgressTm = now;
                }
            });

            nlohmann::json retData;
            retData["task"] = request.task;
            retData["success"] = !resp.empty();
            retData["session"] = resp;

            zeroMqServerClient.sendMessage(
                service::zeromq::ZEROMQ_GCS_UPLOAD_PROGRESS_TOPIC,
                nlohmann::json{
                    {"task", request.task},
                    {"done", true},
                    {"success", !resp.empty()}
                }.dump(),
                true
            );

            zeroMqServerClient.sendMessage(
                service::zeromq::ZEROMQ_RESPOND_GCS_UPLOAD_TOPIC,
                retData.dump()
            );
        });
        t.detach();
        */
    });

    LOG_INFO("Add local recording handlers..." << std::endl);
    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_REQUEST_FOLDER_SIZE_TOPIC, [&zeroMqServerClient](const std::string& msg){
        LOG_INFO("RECEIVE FOLDER SIZE REQUEST: " << msg << std::endl);
        std::thread t([&zeroMqServerClient, msg](){
            const auto json = nlohmann::json::parse(msg);
            const auto request = service::system::GenericIpcRequest<std::string>::fromJson(json);

            service::system::GenericIpcResponse<double> resp;
            resp.task = request.task;
            try {
                resp.data = static_cast<double>(shared::filesystem::getFolderSizeBytes(fs::path(request.data))) / 1024.0 / 1024.0 / 1024.0;
                resp.success = true;
            } catch (std::exception& ex) {
                LOG_WARNING("Failed to compute folder size: " << ex.what() << std::endl);
                resp.success = false;
            }

            zeroMqServerClient.sendMessage(
                service::zeromq::ZEROMQ_RESPOND_FOLDER_SIZE_TOPIC,
                resp.toJson().dump()
            );
        });
        t.detach();
    });

    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_REQUEST_CHANGE_RECORDING_FOLDER_TOPIC, [&zeroMqServerClient](const std::string& msg){
        LOG_INFO("RECEIVE CHANGE RECORDING FOLDER: " << msg << std::endl);
        std::thread t([&zeroMqServerClient, msg](){
            const auto json = nlohmann::json::parse(msg);
            const auto request = service::system::GenericIpcRequest<shared::filesystem::ChangeLocalRecordRequest>::fromJson(json);

            const std::wstring fromFolder = shared::strings::utf8ToWcs(request.data.from);
            const std::wstring toFolder = shared::strings::utf8ToWcs(request.data.to);

            service::system::GenericIpcResponse<void> resp;
            resp.task = request.task;
            try {
                resp.success = shared::filesystem::changeLocalRecordLocation(fs::path(fromFolder), fs::path(toFolder));
            } catch (std::exception& ex) {
                LOG_WARNING("Failed to change local recording folder: " << ex.what() << std::endl);
                resp.success = false;
            }

            zeroMqServerClient.sendMessage(
                service::zeromq::ZEROMQ_RESPOND_CHANGE_RECORDING_FOLDER_TOPIC,
                resp.toJson().dump()
            );
        });
        t.detach();
    });

    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_REQUEST_CLEANUP_RECORDING_FOLDER_TOPIC, [&zeroMqServerClient](const std::string& msg){
        LOG_INFO("RECEIVE CLEANUP RECORDING FOLDER: " << msg << std::endl);
        std::thread t([&zeroMqServerClient, msg](){
            const auto json = nlohmann::json::parse(msg);
            const auto request = service::system::GenericIpcRequest<shared::filesystem::CleanupLocalRecordRequest>::fromJson(json);

            service::system::GenericIpcResponse<void> resp;
            resp.task = request.task;
            try {
                resp.success = shared::filesystem::cleanupLocalRecordLocation(fs::path(request.data.loc), request.data.limit);
            } catch (std::exception& ex) {
                LOG_WARNING("Failed to cleanup local recording folder: " << ex.what() << std::endl);
                resp.success = false;
            }

            zeroMqServerClient.sendMessage(
                service::zeromq::ZEROMQ_RESPOND_CLEANUP_RECORDING_FOLDER_TOPIC,
                resp.toJson().dump()
            );
        });
        t.detach();
    });

    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_REQUEST_VOD_DOWNLOAD_TOPIC, [&zeroMqServerClient](const std::string& msg){
        LOG_INFO("RECEIVE VOD DOWNLOAD REQUEST: " << msg << std::endl);
        std::thread t([&zeroMqServerClient, msg](){
            const auto json = nlohmann::json::parse(msg);
            const auto request = service::system::GenericIpcRequest<std::string>::fromJson(json);
            const auto settings = service::system::getCurrentSettings();
            bool success = false;

            try {
                const auto vodAssoc = service::api::getGlobalApi()->getVod(request.data);

                shared::filesystem::LocalRecordingIndexEntry entry;
                entry.uuid = request.data;
                entry.filename = "video.mp4";
                entry.startTm = vodAssoc.startTime;
                entry.endTm = vodAssoc.endTime;
                entry.cacheTm = shared::nowUtc();
                entry.diskBytes = 0;

                shared::filesystem::LocalRecordingIndexDb::singleton()->initializeFromFolder(settings->recording().localRecordingLocation);
                
                shared::TimePoint lastProgressTm = shared::nowUtc();
                shared::filesystem::LocalRecordingIndexDb::singleton()->addLocalEntryFromUri(service::api::getGlobalApi()->getVodUri(request.data), entry, [&zeroMqServerClient, &lastProgressTm, request](size_t dl, size_t total){
                    const auto now = shared::nowUtc();
                    const auto threshold = lastProgressTm + std::chrono::seconds(1);
                    if (now > threshold) {
                        zeroMqServerClient.sendMessage(
                            service::zeromq::ZEROMQ_VOD_DOWNLOAD_PROGRESS_TOPIC,
                            nlohmann::json{
                                {"task", request.data},
                                {"download", dl},
                                {"total", total},
                                {"done", false}
                            }.dump(),
                            true
                        );
                        lastProgressTm = now;
                    }
                });

                shared::filesystem::LocalRecordingIndexDb::singleton()->cleanupLocalFolder(settings->recording().maxLocalRecordingSizeGb);

                zeroMqServerClient.sendMessage(
                    service::zeromq::ZEROMQ_VOD_DOWNLOAD_PROGRESS_TOPIC,
                    nlohmann::json{
                        {"task", request.data},
                        {"sucess", true},
                        {"done", true}
                    }.dump()
                );
            } catch (std::exception& ex) {
                LOG_WARNING("Failed to download VOD: " << ex.what() << std::endl);
                zeroMqServerClient.sendMessage(
                    service::zeromq::ZEROMQ_VOD_DOWNLOAD_PROGRESS_TOPIC,
                    nlohmann::json{
                        {"task", request.data},
                        {"sucess", false},
                        {"done", true}
                    }.dump()
                );
            }
        });
        t.detach();
    });

    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_REQUEST_LOCAL_VOD_TOPIC, [&zeroMqServerClient](const std::string& msg){
        LOG_INFO("RECEIVE FIND LOCAL VOD REQUEST: " << msg << std::endl);
        std::thread t([&zeroMqServerClient, msg](){
            const auto json = nlohmann::json::parse(msg);
            const auto request = service::system::GenericIpcRequest<std::string>::fromJson(json);
            const auto settings = service::system::getCurrentSettings();
            const auto singleton = shared::filesystem::LocalRecordingIndexDb::singleton();

            service::system::GenericIpcResponse<std::string> resp;
            resp.task = request.task;
            try {
                singleton->initializeFromFolder(settings->recording().localRecordingLocation);
                const auto entry = singleton->getEntryForUuid(request.data);

                if (entry.has_value()) {
                    resp.data = shared::filesystem::pathUtf8(singleton->getEntryPath(entry.value()));
                    resp.success = true;
                } else {
                    resp.success = false;
                }
            } catch (std::exception& ex) {
                LOG_WARNING("Failed to get local VOD: " << ex.what() << std::endl);
                resp.success = false;
            }

            zeroMqServerClient.sendMessage(
                service::zeromq::ZEROMQ_RESPOND_LOCAL_VOD_TOPIC,
                resp.toJson().dump()
            );
        });
        t.detach();
    });

    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_REQUEST_DELETE_LOCAL_VOD_TOPIC, [&zeroMqServerClient](const std::string& msg){
        LOG_INFO("RECEIVE LOCAL VOD DELETE REQUEST: " << msg << std::endl);
        std::thread t([&zeroMqServerClient, msg](){
            const auto json = nlohmann::json::parse(msg);
            const auto request = service::system::GenericIpcRequest<std::string>::fromJson(json);
            const auto settings = service::system::getCurrentSettings();
            const auto singleton = shared::filesystem::LocalRecordingIndexDb::singleton();

            service::system::GenericIpcResponse<std::string> resp;
            resp.task = request.task;
            try {
                singleton->initializeFromFolder(settings->recording().localRecordingLocation);
                singleton->removeLocalEntry(request.data);
                resp.success = true;
            } catch (std::exception& ex) {
                LOG_WARNING("Failed to delete local VOD: " << ex.what() << std::endl);
                resp.success = false;
            }

            zeroMqServerClient.sendMessage(
                service::zeromq::ZEROMQ_RESPOND_DELETE_LOCAL_VOD_TOPIC,
                resp.toJson().dump()
            );
        });
        t.detach();
    });

    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_RELOAD_SETTINGS, [](const std::string& msg){
        LOG_INFO("RECEIVE RELOAD SETTINGS REQUEST." << std::endl);
        service::system::getCurrentSettings()->reloadSettingsFromFile();
        shared::time::NTPClient::singleton()->enable(service::system::getCurrentSettings()->enableNtp(), true);
    });

    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_REQUEST_KEYCODE_CHAR, [&zeroMqServerClient](const std::string& msg){
        LOG_INFO("RECEIVE KEYCODE CHAR REQUEST:" << msg << std::endl);
        const auto json = nlohmann::json::parse(msg);
        auto request = service::system::GenericIpcRequest<int>::fromJson(json);

        service::system::GenericIpcResponse<std::string> resp;
        resp.task = request.task;

        if (request.data >= VK_F13 && request.data <= VK_F24) {
            // Windows apparently doesn't suppor these via GetKeyNameTextW
            std::ostringstream name;
            name << "F" << (13 + request.data - VK_F13);
            resp.success = true;
            resp.data = name.str();
        } else if (request.data == VK_RBUTTON) {
            resp.success = true;
            resp.data = "MOUSE1";
        } else if (request.data == VK_MBUTTON) {
            resp.success = true;
            resp.data = "MOUSE2";
        } else if (request.data == VK_XBUTTON1) {
            resp.success = true;
            resp.data = "MOUSE3";
        } else if (request.data == VK_XBUTTON2) {
            resp.success = true;
            resp.data = "MOUSE4";
        } else {
            UINT scanCode = MapVirtualKeyW(request.data, MAPVK_VK_TO_VSC);
            switch (request.data) {
                case VK_LEFT:
                case VK_UP:
                case VK_RIGHT:
                case VK_DOWN:
                case VK_PRIOR:
                case VK_NEXT:
                case VK_END:
                case VK_HOME:
                case VK_INSERT:
                case VK_DELETE:
                case VK_DIVIDE:
                case VK_NUMLOCK:
                    scanCode |= 0x100;
            }

            wchar_t nameBuffer[256];
            const auto result = GetKeyNameTextW(scanCode << 16, nameBuffer, 256);

            std::wstring nameWStr(nameBuffer);
            std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
            resp.success = (result > 0);
            resp.data = conv.to_bytes(nameWStr);
        }

        zeroMqServerClient.sendMessage(
            service::zeromq::ZEROMQ_RESPOND_KEYCODE_CHAR,
            resp.toJson().dump()
        );
    });

    LOG_INFO("Initializing game preview stream handler..." << std::endl);
    service::recorder::GamePreviewStream previewStream;
    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_START_GAME_RECORDING_STREAM, [&previewStream, &zeroMqServerClient](const std::string& msg){
        LOG_INFO("START GAME RECORDING STREAM:" << msg << std::endl);
        const auto json = nlohmann::json::parse(msg);
        previewStream.start(json["url"].get<std::string>(), static_cast<shared::EGame>(json["game"].get<int>()));
    });

    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_STOP_GAME_RECORDING_STREAM, [&previewStream, &zeroMqServerClient](const std::string& msg){
        LOG_INFO("STOP GAME RECORDING STREAM:" << msg << std::endl);
        previewStream.stop();
    });

    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_ENABLE_PREVIEW_GAME_RECORDING_STREAM, [&previewStream, &zeroMqServerClient](const std::string& msg){
        LOG_INFO("ENABLE PREVIEW GAME RECORDING STREAM:" << msg << std::endl);
        const auto json = nlohmann::json::parse(msg);
        previewStream.enableOverlay(json["enabled"].get<bool>());
    });

    zeroMqServerClient.addHandler(service::zeromq::ZEROMQ_RELOAD_GAME_RECORDING_STREAM, [&previewStream, &zeroMqServerClient](const std::string& msg){
        LOG_INFO("RELOAD GAME RECORDING STREAM:" << msg << std::endl);
        previewStream.reload();
    });

    const auto mode = vm["mode"].as<std::string>();
    if (mode == "") {
        defaultMain();

        LOG_INFO("Setting up windows message loop..." << std::endl);
        shared::system::win32::Win32MessageLoop::singleton()->start();
    } else if (mode == "wow_test") {
        wowTest(vm["log"].as<std::string>(), vm["vod"].as<std::string>(), vm["vodTime"].as<std::string>());
    } else {
        THROW_ERROR("Unknown Mode: " << mode);
    }

    curl_global_cleanup();
    Pa_Terminate();

#ifdef _WIN32
    CoUninitialize();
#endif
    return 0;
}