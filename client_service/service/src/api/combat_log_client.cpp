#include "api/combat_log_client.h"
#include "api/squadov_api.h"
#include "shared/log/log.h"

#include <nlohmann/json.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>

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
    _signer = service::api::getAwsApi()->createSigner("apigateway");
}

CombatLogClient::~CombatLogClient() {
    _running = false;
    if (_uploadThread.joinable()) {
        _uploadThread.join();
    }
}

void CombatLogClient::setMetadata(const std::string& key, const std::string& value) {
    std::lock_guard guard(_mutex);
    _metadata[key] = value;
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
            {
                std::unique_lock lk(_mutex);
                const auto bufferIsFull = _cv.wait_for(lk, std::chrono::milliseconds(250), [this]{return isBufferFull();});

                // Empty buffers can be ignored.
                if (_buffer.empty()) {
                    continue;
                }

                const auto now = shared::nowUtc();
                // If the buffer isn't full then we want to check to see if the timeout happened and only in the case
                // where the timeout hasn't happened do we want to skip and wait again.
                if (!bufferIsFull) {
                    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastUploadTime);
                    if (elapsedMs < _timeout) {
                        continue;
                    }
                }

                // We need to send this buffer along with the metadata associated with this buffer in a compressed format.
                // Note that it's just convenient to use JSON plus compression on top instead of a fancy format like AVRO.
                // We also need to read all this data while we still have the lock.
                for (const auto& kvp: _metadata) {
                    dataToSend[kvp.first] = kvp.second;
                }

                dataToSend["logs"] = nlohmann::json::array();
                for (const auto& ln: _buffer) {
                    dataToSend["logs"].push_back(ln);
                }
                
                _buffer.clear();
                _lastUploadTime = now;
            }

            // Now that we're *outside* of the lock, we can do more computationally/time expensive operations
            // like compression and HTTP requests. This way any producers that need to continue to add log lines
            // to the combat log client can do so without having to wait for compression and HTTP requests to finish
            // since at this point we're no longer touching the raw buffer, but rather the JSON structure that we want
            // to send.
            compressedBuffer.clear();

            {
                auto compressedDevice = boost::iostreams::back_inserter(compressedBuffer);
                boost::iostreams::filtering_ostream outputStream;
                outputStream.push(boost::iostreams::zlib_compressor{});
                outputStream.push(compressedDevice);
                outputStream << dataToSend.dump() << std::flush;
                outputStream.pop();
            }


            for (int i = 0; i < 3; ++i) {
                Aws::Http::URI uri;
                uri.SetScheme(Aws::Http::Scheme::HTTPS);
                uri.SetAuthority(Aws::String(_hostname));
                uri.AddPathSegments(_endpointPath);

                // Finally, HTTP POST the data up to our servers. Our servers will take care of putting it onto a proper
                // queue for processing. We use the AWS API to sign the request since we use AWS IAM as the way to authenticate requests.
                // Since we know we're using AWS we can just directly create AWS HTTP requests and sign them and send them up.
                auto request = std::make_shared<Aws::Http::Standard::StandardHttpRequest>(
                    uri,
                    Aws::Http::HttpMethod::HTTP_POST
                );
                
                if (!_signer->SignRequest(*request)) {
                    LOG_WARNING("Failed to sign combat log POST request." << std::endl);
                    continue;
                }

                const auto response = _httpClient->MakeRequest(request);
                if (!response || response->HasClientError() || response->GetResponseCode() == Aws::Http::HttpResponseCode::OK) {
                    LOG_WARNING("Failed to make combat log POST request." << std::endl);
                    continue;
                }

                break;
            }
        }
    });
}

bool CombatLogClient::isBufferFull() const {
    return _buffer.size() > _maxBufferSize;
}

}