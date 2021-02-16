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
const std::string ZEROMQ_CHANGE_PAUSE_TOPIC = "change-state-pause";
const std::string ZEROMQ_RUNNING_GAMES_TOPIC = "change-running-games";
const std::string ZEROMQ_RECORDING_GAMES_TOPIC = "change-recording-games";
const std::string ZEROMQ_REQUEST_AUDIO_INPUT_TOPIC = "request-audio-input";
const std::string ZEROMQ_RESPOND_AUDIO_INPUT_TOPIC = "respond-input-devices";
const std::string ZEROMQ_REQUEST_AUDIO_OUTPUT_TOPIC = "request-audio-output";
const std::string ZEROMQ_RESPOND_AUDIO_OUTPUT_TOPIC = "respond-output-devices";
const std::string ZEROMQ_REQUEST_VOD_CLIP_TOPIC = "request-vod-clip";
const std::string ZEROMQ_RESPOND_VOD_CLIP_TOPIC = "respond-vod-clip";

using ZeroMQHandler = std::function<void(const std::string&)>;

// We do assume that we'll be only running this backend service in the presence of a frontend.
// Use this class to communicate with said frontend via ZeroMQ.
class ZeroMQServerClient {
public:
    ZeroMQServerClient();
    ~ZeroMQServerClient();

    void start();
    void addHandler(const std::string& topic, const ZeroMQHandler& handler);
    void sendMessage(const std::string& topic, const std::string& message);

private:
    zmq::context_t _ctx;
    zmq::socket_t _pub;
    zmq::socket_t _sub;
    bool _running = true;
    std::thread _subThread;
    std::unordered_map<std::string, std::vector<ZeroMQHandler>> _handlers;
};

}