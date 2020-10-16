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