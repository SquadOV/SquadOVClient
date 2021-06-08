#include "api/local_api.h"
#include "shared/log/log.h"
#include "shared/env.h"
#include "shared/filesystem/local_record.h"
#include "api/local/dto/local_vods_dto.h"
#include "system/settings.h"

#include <oatpp/web/server/HttpConnectionHandler.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <vector>

namespace service::api {
namespace {

class ListLocalVodsHandler: public oatpp::web::server::HttpRequestHandler {
    OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, _mapper);
public:
    std::shared_ptr<OutgoingResponse> handle(const std::shared_ptr<IncomingRequest>& request) override {
        const auto start = std::stoi(request->getQueryParameter("start")->std_str());
        const auto end = std::stoi(request->getQueryParameter("end")->std_str());

        auto message = service::api::local::dto::LocalVodsDto::createShared();

        auto db = shared::filesystem::LocalRecordingIndexDb::singleton();
        const auto settings = service::system::getCurrentSettings();
        db->initializeFromFolder(settings->recording().localRecordingLocation);

        message->data = {};
        const auto local = db->getAllLocalEntries();
        for (auto i = start; i < end; ++i) {
            if (i < 0 || i >= static_cast<int>(local.size())) {
                continue;
            }
            
            const auto entry = local[i];
            message->data->push_back(
                oatpp::base::StrBuffer::createFromCString(entry.uuid.c_str())
            );
        }

        return ResponseFactory::createResponse(Status::CODE_200, message, _mapper);
    }
};

}

LocalApiServer::LocalApiServer() {
    oatpp::base::Environment::init();
}

LocalApiServer::~LocalApiServer() {
    _server->stop();
    if (_serverRunThread.joinable()) {
        _serverRunThread.join();
    }

    oatpp::base::Environment::destroy();
}

void LocalApiServer::start() {
    _components = std::make_shared<local::AppComponent>();

    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
    router->route("GET", "/api/vods", std::make_shared<ListLocalVodsHandler>());

    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

    _server = std::make_shared<oatpp::network::Server>(connectionProvider, connectionHandler);
    _serverRunThread = std::thread([this](){
        _server->run();
    });

    const auto port = std::stoi(std::string(reinterpret_cast<char*>(connectionProvider->getProperty("port").getData())));

    _port = port;
    LOG_INFO("Running Local Service API on Port " << _port << std::endl);
}

}