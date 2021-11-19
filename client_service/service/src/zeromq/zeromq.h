#pragma once

#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <zmq.hpp>

namespace service::zeromq {

const std::string ZEROMQ_SESSION_ID_TOPIC = "session-id";
const std::string ZEROMQ_READY_TOPIC = "on-ready";
const std::string ZEROMQ_LOCAL_API_PORT_TOPIC = "on-local-api-port";
const std::string ZEROMQ_CHANGE_PAUSE_TOPIC = "change-state-pause";
const std::string ZEROMQ_RUNNING_GAMES_TOPIC = "change-running-games";
const std::string ZEROMQ_RECORDING_GAMES_TOPIC = "change-recording-games";
const std::string ZEROMQ_NOTIFICATION_POPUP_POST_GAME = "post-game-notification";
const std::string ZEROMQ_REQUEST_AUDIO_INPUT_TOPIC = "request-audio-input";
const std::string ZEROMQ_RESPOND_AUDIO_INPUT_TOPIC = "respond-input-devices";
const std::string ZEROMQ_REQUEST_AUDIO_OUTPUT_TOPIC = "request-audio-output";
const std::string ZEROMQ_RESPOND_AUDIO_OUTPUT_TOPIC = "respond-output-devices";
const std::string ZEROMQ_REQUEST_VOD_CLIP_TOPIC = "request-vod-clip";
const std::string ZEROMQ_RESPOND_VOD_CLIP_TOPIC = "respond-vod-clip";
const std::string ZEROMQ_REQUEST_CLOUD_UPLOAD_TOPIC = "request-cloud-upload";
const std::string ZEROMQ_RESPOND_CLOUD_UPLOAD_TOPIC = "respond-cloud-upload";
const std::string ZEROMQ_CLOUD_UPLOAD_PROGRESS_TOPIC = "cloud-upload-progress";
const std::string ZEROMQ_REQUEST_FOLDER_SIZE_TOPIC = "request-folder-size";
const std::string ZEROMQ_RESPOND_FOLDER_SIZE_TOPIC = "respond-folder-size";
const std::string ZEROMQ_REQUEST_CHANGE_RECORDING_FOLDER_TOPIC = "request-change-recording-folder";
const std::string ZEROMQ_RESPOND_CHANGE_RECORDING_FOLDER_TOPIC = "respond-change-recording-folder";
const std::string ZEROMQ_REQUEST_CLEANUP_RECORDING_FOLDER_TOPIC = "request-cleanup-recording-folder";
const std::string ZEROMQ_RESPOND_CLEANUP_RECORDING_FOLDER_TOPIC = "respond-cleanup-recording-folder";
const std::string ZEROMQ_REQUEST_VOD_DOWNLOAD_TOPIC = "request-vod-download";
const std::string ZEROMQ_VOD_DOWNLOAD_PROGRESS_TOPIC = "vod-download-progress";
const std::string ZEROMQ_REQUEST_LOCAL_VOD_TOPIC = "request-check-local-vod";
const std::string ZEROMQ_RESPOND_LOCAL_VOD_TOPIC = "respond-check-local-vod";
const std::string ZEROMQ_REQUEST_DELETE_LOCAL_VOD_TOPIC = "request-delete-local-vod";
const std::string ZEROMQ_RESPOND_DELETE_LOCAL_VOD_TOPIC = "respond-delete-local-vod";
const std::string ZEROMQ_RELOAD_SETTINGS = "reload-settings";
const std::string ZEROMQ_REQUEST_KEYCODE_CHAR = "request-keycode-char";
const std::string ZEROMQ_RESPOND_KEYCODE_CHAR = "respond-keycode-char";
const std::string ZEROMQ_START_GAME_RECORDING_STREAM = "start-game-recording-stream";
const std::string ZEROMQ_STOP_GAME_RECORDING_STREAM = "stop-game-recording-stream";
const std::string ZEROMQ_ENABLE_PREVIEW_GAME_RECORDING_STREAM = "enable-preview-game-recording-stream";
const std::string ZEROMQ_RELOAD_GAME_RECORDING_STREAM = "reload-game-recording-stream";
const std::string ZEROMQ_PREVIEW_STATUS_MESSAGE = "preview-stream-status";

using ZeroMQHandler = std::function<void(const std::string&)>;

// We do assume that we'll be only running this backend service in the presence of a frontend.
// Use this class to communicate with said frontend via ZeroMQ.
class ZeroMQServerClient {
public:
    ZeroMQServerClient();
    ~ZeroMQServerClient();

    void start();
    void addHandler(const std::string& topic, const ZeroMQHandler& handler);
    void sendMessage(const std::string& topic, const std::string& message, bool quiet = false);

private:
    zmq::context_t _ctx;
    zmq::socket_t _pub;
    zmq::socket_t _sub;
    bool _running = true;
    std::thread _subThread;
    std::unordered_map<std::string, std::vector<ZeroMQHandler>> _handlers;
};

}