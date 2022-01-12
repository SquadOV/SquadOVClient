#include "api/combat_log_client.h"
#include "api/squadov_api.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"
#include "shared/base64/encode.h"

#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>

#include <sstream>

#include <aws/core/http/standard/StandardHttpRequest.h>
#include <aws/core/http/URI.h>

namespace service::api {

std::string combatLogEndpointToPath(CombatLogEndpoint ep) {
    switch (ep) {
        case CombatLogEndpoint::Ff14:
            return "/ff14";
    }
    return "";
}

CombatLogClient::CombatLogClient(CombatLogEndpoint endpoint):
    _endpoint(endpoint)
{
    _endpointPath = combatLogEndpointToPath(_endpoint);
    LOG_INFO("Combat Logging to Path: " << _endpointPath);

    _hostname = service::api::getGlobalApi()->getCombatLogApiConfiguration();
    LOG_INFO("Combat Logging to Hostname: " << _hostname << std::endl);

    _httpClient = service::api::getAwsApi()->createHttpClient();
    _signer = service::api::getAwsApi()->createSigner("execute-api");
}

CombatLogClient::~CombatLogClient() {
    flush();
}

void CombatLogClient::flush() {
    _running = false;
    if (_uploadThread.joinable()) {
        _uploadThread.join();
    }
}

void CombatLogClient::setMetadata(const std::string& key, const std::string& value) {
    std::lock_guard guard(_mutex);
    _metadata[key] = value;
}

void CombatLogClient::setPartitionId(const std::string& id) {
    std::lock_guard guard(_mutex);
    _partitionId = id;
    _sequenceId = 0;
}

void CombatLogClient::addLine(const std::string& line) {
    std::lock_guard guard(_mutex);
    _buffer.push_back(line);
    if (isBufferFull()) {
        _cv.notify_all();
    }
}

void CombatLogClient::start() {
    _running = true;
    _uploadThread = std::thread([this](){
        std::vector<std::string> workingBuffer;
        workingBuffer.reserve(_maxBufferSize);

        std::vector<char> compressedBuffer;
        while (_running) {
            nlohmann::json dataToSend;
            if (!populateData(dataToSend, true)) {
                continue;
            }

            // Now that we're *outside* of the lock, we can do more computationally/time expensive operations
            // like compression and HTTP requests. This way any producers that need to continue to add log lines
            // to the combat log client can do so without having to wait for compression and HTTP requests to finish
            // since at this point we're no longer touching the raw buffer, but rather the JSON structure that we want
            // to send.
            compressedBuffer.clear();
            sendData(dataToSend, compressedBuffer);
        }

        nlohmann::json flushData;
        if (populateData(flushData, false)) {
            sendData(flushData, compressedBuffer);
        }
    });
}

bool CombatLogClient::isBufferFull() const {
    return _buffer.size() > _maxBufferSize;
}

bool CombatLogClient::populateData(nlohmann::json& data, bool wait) {
    std::unique_lock lk(_mutex);
    const auto bufferIsFull = wait ? _cv.wait_for(lk, std::chrono::milliseconds(250), [this]{return isBufferFull();}) : true;

    // Empty buffers can be ignored.
    if (_buffer.empty()) {
        return false;
    }

    const auto now = shared::nowUtc();
    // If the buffer isn't full then we want to check to see if the timeout happened and only in the case
    // where the timeout hasn't happened do we want to skip and wait again.
    if (!bufferIsFull) {
        const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastUploadTime);
        if (elapsedMs < _timeout) {
            return false;
        }
    }

    // We need to send this buffer along with the metadata associated with this buffer in a compressed format.
    // Note that it's just convenient to use JSON plus compression on top instead of a fancy format like AVRO.
    // We also need to read all this data while we still have the lock.
    for (const auto& kvp: _metadata) {
        data[kvp.first] = kvp.second;
    }

    data["logs"] = nlohmann::json::array();
    for (const auto& ln: _buffer) {
        data["logs"].push_back(ln);
    }
    
    _buffer.clear();
    _lastUploadTime = now;
    return true;
}

void CombatLogClient::sendData(const nlohmann::json& data, std::vector<char>& buffer) {
    {
        auto compressedDevice = boost::iostreams::back_inserter(buffer);
        boost::iostreams::filtering_ostream outputStream;
        outputStream.push(boost::iostreams::zlib_compressor{});
        outputStream.push(compressedDevice);
        outputStream << data.dump() << std::flush;
        outputStream.pop();
    }

    const auto seqId = _sequenceId++;
    for (int i = 0; i < 3; ++i) {
        std::stringstream uri;
        uri << "https://" << _hostname  << _endpointPath;

        // Finally, HTTP POST the data up to our servers. Our servers will take care of putting it onto a proper
        // queue for processing. We use the AWS API to sign the request since we use AWS IAM as the way to authenticate requests.
        // Since we know we're using AWS we can just directly create AWS HTTP requests and sign them and send them up.
        auto request = std::make_shared<Aws::Http::Standard::StandardHttpRequest>(
            Aws::Http::URI(uri.str()),
            Aws::Http::HttpMethod::HTTP_PUT
        );

        request->SetHeaderValue("partition", Aws::String(_partitionId));
        request->SetHeaderValue("sequence", Aws::String(std::to_string(seqId)));

        auto bodyStream = std::make_shared<std::stringstream>();

        // For Kinesis-PutRecord, the data needs to be base64-encoded and not over 1MB.
        // I'm going to assume that we're never going to run into a situation where the data we send is greater than 1MB.
        // Knock on wood.
        *bodyStream << shared::base64::encode(std::string_view(buffer.data(), buffer.size()), shared::base64::BASE64_ENCODE_DEFAULT_CHARSET);
        request->AddContentBody(bodyStream);

        {
            bodyStream->seekg(0, bodyStream->end);
            const auto streamSize = bodyStream->tellg();
            bodyStream->seekg(0, bodyStream->beg);
            request->SetContentLength(Aws::String(std::to_string(streamSize)));
        }

        request->SetResponseStreamFactory(Aws::Utils::Stream::DefaultResponseStreamFactoryMethod);
        
        if (!_signer->SignRequest(*request)) {
            LOG_WARNING("Failed to sign combat log PUT request." << std::endl);
            continue;
        }

        const auto response = _httpClient->MakeRequest(request);
        if (!response || response->HasClientError() || response->GetResponseCode() != Aws::Http::HttpResponseCode::OK) {
            LOG_WARNING("Failed to make combat log PUT request." << std::endl);

            std::ostringstream tmp;
            tmp << response->GetResponseBody().rdbuf();
            LOG_WARNING("...Body: " << tmp.str() << std::endl);
            LOG_WARNING("...Err: " << response->GetClientErrorMessage() << std::endl);
            continue;
        }

        break;
    }
}

}