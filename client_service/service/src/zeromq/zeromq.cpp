#include "zeromq/zeromq.h"
#include "shared/env.h"
#include "shared/log/log.h"

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
        LOG_INFO("Use ZeroMQ with Server Key" << std::endl);
        _pub.set(zmq::sockopt::curve_serverkey, serverKey);
        generateCurveKeyPairForSocket(_pub);

        _sub.set(zmq::sockopt::curve_serverkey, serverKey);
        generateCurveKeyPairForSocket(_sub);
    }

    _sub.set(zmq::sockopt::subscribe, ZEROMQ_SESSION_ID_TOPIC);
    _sub.set(zmq::sockopt::subscribe, ZEROMQ_CHANGE_PAUSE_TOPIC);
    _sub.set(zmq::sockopt::subscribe, ZEROMQ_REQUEST_AUDIO_INPUT_TOPIC);
    _sub.set(zmq::sockopt::subscribe, ZEROMQ_REQUEST_AUDIO_OUTPUT_TOPIC);
    _sub.set(zmq::sockopt::subscribe, ZEROMQ_REQUEST_VOD_CLIP_TOPIC);
    _sub.set(zmq::sockopt::subscribe, ZEROMQ_REQUEST_GCS_UPLOAD_TOPIC);
    _sub.set(zmq::sockopt::subscribe, ZEROMQ_REQUEST_FOLDER_SIZE_TOPIC);
    _sub.set(zmq::sockopt::subscribe, ZEROMQ_REQUEST_CHANGE_RECORDING_FOLDER_TOPIC);
    _sub.set(zmq::sockopt::subscribe, ZEROMQ_REQUEST_CLEANUP_RECORDING_FOLDER_TOPIC);
    _sub.set(zmq::sockopt::subscribe, ZEROMQ_REQUEST_VOD_DOWNLOAD_TOPIC);
     // Return immediately so that the infinite loop can exit out when needed.
    _sub.set(zmq::sockopt::rcvtimeo, 0);

    {
        std::ostringstream host;
        host << "tcp://127.0.0.1:" << shared::getEnv("SQUADOV_ZEROMQ_SERVICE_PORT", "60001");
        _pub.connect(host.str());
        LOG_INFO("Publish ZeroMQ on: " << host.str() << std::endl);
    }

    {
        std::ostringstream host;
        host << "tcp://127.0.0.1:" << shared::getEnv("SQUADOV_ZEROMQ_UI_PORT", "60000");
        _sub.connect(host.str());
        LOG_INFO("Subscribe ZeroMQ on: " << host.str() << std::endl);
    }
    
}

ZeroMQServerClient::~ZeroMQServerClient() {
    _running = false;
    _subThread.join();
    //_pub.unbind(_pub.get(zmq::sockopt::last_endpoint));
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
    auto ret = zmq::send_multipart(_pub, buffer);
    LOG_INFO("Send ZeroMQ Message: " << topic << "\t" << ret.value_or(0) << std::endl);
}

}