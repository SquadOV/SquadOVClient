#include "http/http_client.h"

#include "shared/constants.h"

#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <thread>

namespace service::http {
namespace {

size_t curlStringStreamCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::ostringstream& buffer = *reinterpret_cast<std::ostringstream*>(userdata);
    buffer << std::string(ptr, nmemb);
    return nmemb;
}

class HttpRequest {
public:
    HttpRequest(const std::string& uri, const Headers& headers) {
        _curl = curl_easy_init();
        std::cout << "CURL REQUEST TO: " << uri << std::endl;
        curl_easy_setopt(_curl, CURLOPT_URL, uri.c_str());
    
        for (const auto& [key, value] : headers) {
            std::ostringstream h;
            h << key << ": " << value;
            _headers = curl_slist_append(_headers, h.str().c_str());
        }

        curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _headers);
    }

    ~HttpRequest() {
        curl_easy_cleanup(_curl);
        curl_slist_free_all(_headers);
    }

    HttpResponsePtr Do() {
        auto resp = std::make_unique<HttpResponse>();

        std::ostringstream buffer;
        curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, curlStringStreamCallback);

        resp->curlError = curl_easy_perform(_curl);
        if (resp->curlError == CURLE_OK) {
            curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &resp->status);
        }

        resp->body = buffer.str();
        return resp;
    }

private:
    CURL* _curl = nullptr;
    curl_slist* _headers = nullptr;
};

}

HttpClient::HttpClient(const std::string& baseUri):
    _baseUri(baseUri) {
    _lastRequestTime = shared::nowUtc();
}

HttpClient::~HttpClient() {
}

void HttpClient::setBearerAuthToken(const std::string& token) {
    std::ostringstream str;
    str << "Bearer " << token;
    _headers["Authorization"] = str.str();
}

void HttpClient::setHeaderKeyValue(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

HttpResponsePtr HttpClient::Get(const std::string& path) const {
    tickRateLimit();

    std::ostringstream fullPath;
    fullPath << _baseUri << path;

    HttpRequest req(fullPath.str(), _headers);
    return req.Do();
}

void HttpClient::tickRateLimit() const {
    if (_secondsPerTask < shared::EPSILON) {
        return;
    }

    const auto now = shared::nowUtc();

    // First check if we need to sleep for some time before executing the request.
    bool needLimit = false;
    std::chrono::milliseconds sleepDuration;
    {
        std::shared_lock<std::shared_mutex> guard(_rateLimitMutex);
        const auto elapsedSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastRequestTime).count() / 1000.0;
        needLimit = (elapsedSeconds < _secondsPerTask);
        sleepDuration = std::chrono::milliseconds(static_cast<long long>((_secondsPerTask - elapsedSeconds) * 1000.0));
    }
    
    if (needLimit) {
        std::this_thread::sleep_for(sleepDuration);
    }

    std::unique_lock<std::shared_mutex> guard(_rateLimitMutex);
    _lastRequestTime = shared::nowUtc();
}

}