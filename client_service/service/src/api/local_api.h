#pragma once

#include "api/local/app_component.h"

#include <oatpp/core/macro/component.hpp>
#include <oatpp/network/Server.hpp>

#include <memory>
#include <thread>

namespace service::api {

class LocalApiServer {
public:
    LocalApiServer();
    ~LocalApiServer();

    void start();
    int port() const { return _port; }

private:
    std::shared_ptr<local::AppComponent> _components;
    std::shared_ptr<oatpp::network::Server> _server;
    std::thread _serverRunThread;
    int _port = 0;
};

}