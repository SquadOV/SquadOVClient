#pragma once

#include <chrono>
#include <condition_variable>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "shared/time.h"
#include "shared/http/http_client.h"
#include "shared/json.h"
#include "api/aws_api.h"

namespace service::api {

enum class CombatLogEndpoint {
    Ff14,
    Wow
};

std::string combatLogEndpointToPath(CombatLogEndpoint ep);

class CombatLogClient {
public:
    explicit CombatLogClient(CombatLogEndpoint endpoint);
    ~CombatLogClient();

    // These functions must be called before start().
    void setMaxBufferSize(size_t sz) { _maxBufferSize = sz; }
    void setTimeout(const std::chrono::seconds& s) { _timeout = s; }

    // Any additional data that must be sent along with each batch of combat logs
    // (e.g. the match uuid, etc.) should be set as metadata.
    void setMetadata(const std::string& key, const std::string& value);
    void setPartitionId(const std::string& id, const shared::TimePoint& tm);
    void setCombatLogState(const nlohmann::json& state) { _clState = state; }

    void addLine(const std::string& line);
    void start();
    void flush();
private:
    CombatLogEndpoint _endpoint;
    std::string _endpointPath;

    std::vector<std::string> _buffer;
    std::thread _uploadThread;
    bool _running = false;  
    std::unordered_map<std::string, std::string> _metadata;
    std::string _hostname;

    bool isBufferFull() const;
    bool populateData(nlohmann::json& data, bool wait);
    void sendData(const nlohmann::json& data, std::vector<char>& buffer);

    // We need to control access to the buffer.
    mutable std::mutex _mutex;
    // Use a condition variable to make signalling that the buffer is full easier.
    std::condition_variable _cv;

    // Number of elements to store in _buffer before we send a batch to the server.
    size_t _maxBufferSize = 1000;
    // Amount of time that elapsed before we force the buffer to clear out and send data.
    std::chrono::seconds _timeout = std::chrono::seconds(5);

    // 
    shared::TimePoint _lastUploadTime = shared::zeroTime();

    std::shared_ptr<Aws::Http::HttpClient> _httpClient;
    std::shared_ptr<Aws::Client::AWSAuthV4Signer> _signer;

    int64_t _sequenceId = 0;
    std::string _partitionId;
    shared::TimePoint _startTime;

    nlohmann::json _clState;
};

using CombatLogClientPtr = std::unique_ptr<CombatLogClient>;

}