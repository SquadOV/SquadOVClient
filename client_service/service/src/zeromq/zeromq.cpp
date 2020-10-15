#include "zeromq/zeromq.h"
#include "shared/env.h"

#include <chrono>
#include <sstream>
#include <zmq_addon.hpp>

#include <iostream>

using namespace std::chrono_literals;

namespace service::zeromq {
namespace {

void generateCurveKeyPairForSocket(zmq::socket_t& socket) {
    char publicKey[41];
    char secretKey[41];
    zmq_curve_keypair(publicKey, secretKey);

    socket.set(zmq::sockopt::curve_publickey, std::string(publicKey));
    socket.set(zmq::sockopt::curve_secretkey, std::string(secretKey));
}

}

ZeroMQServerClient::ZeroMQServerClient() {
    _pub = zmq::socket_t(_ctx, zmq::socket_type::pub);
    _sub = zmq::socket_t(_ctx, zmq::socket_type::sub);

    const std::string serverKey = shared::getEnv("SQUADOV_ZEROMQ_SERVER_KEY", "");
    if (!serverKey.empty()) {
        _pub.set(zmq::sockopt::curve_serverkey, serverKey);
        generateCurveKeyPairForSocket(_pub);

        _sub.set(zmq::sockopt::curve_serverkey, serverKey);
        generateCurveKeyPairForSocket(_sub);
    }

    _sub.set(zmq::sockopt::subscribe, ZEROMQ_SESSION_ID_TOPIC);
     // Return immediately so that the infinite loop can exit out when needed.
    _sub.set(zmq::sockopt::rcvtimeo, 0);

    std::ostringstream host;
    host << "tcp://127.0.0.1:" << shared::getEnv("SQUADOV_ZEROMQ_PORT", "60000");
    _pub.connect(host.str());
    _sub.connect(host.str());
    std::cout << "PORT: " << host.str() << std::endl;
}

ZeroMQServerClient::~ZeroMQServerClient() {
    _running = false;
    _subThread.join();
    _pub.close();
    _sub.close();
    _ctx.close();
}

void ZeroMQServerClient::start() {
    // Start a new thread to start listening for subscriber messages.
    _subThread = std::thread([this](){
        std::vector<zmq::message_t> messages;
        while (_running) {
            const auto ret = zmq::recv_multipart(_sub, std::back_inserter(messages));
            // TODO: Support variable number of messages here?
            if (ret.has_value() && ret == 2) {
                // I *believe* the first message should be the topic.
                const auto topic = messages[0].to_string();
                const auto message = messages[1].to_string();

                for (const auto& h : _handlers[topic]) {
                    h(message);
                }
            }

            messages.clear();
            std::this_thread::sleep_for(100ms);
        }
    });
}

void ZeroMQServerClient::addHandler(const std::string& topic, const ZeroMQHandler& handler) {
    _handlers[topic].push_back(handler);
}

void ZeroMQServerClient::sendMessage(const std::string& topic, const std::string& message) {
    std::array<zmq::const_buffer, 2> buffer = {
        zmq::buffer(topic),
        zmq::buffer(message)
    };
    zmq::send_multipart(_pub, buffer);
}

}