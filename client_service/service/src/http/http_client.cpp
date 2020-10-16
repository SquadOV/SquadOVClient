#include "http/http_client.h"

#include "shared/constants.h"
#include "shared/log/log.h"

#include <boost/algorithm/string.hpp>
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <thread>

namespace service::http {

class HttpRequest {
public:
    HttpRequest(const std::string& uri, const Headers& headers, bool allowSelfSigned);
    ~HttpRequest();
    HttpResponsePtr execute();

    void doDelete();
    void doPost(const nlohmann::json& body);

private:
    CURL* _curl = nullptr;
    curl_slist* _headers = nullptr;
};

size_t curlStringStreamCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::ostringstream& buffer = *reinterpret_cast<std::ostringstream*>(userdata);
    buffer << std::string(ptr, nmemb);
    return nmemb;
}

size_t curlHeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    HttpResponse* resp = reinterpret_cast<HttpResponse*>(userdata);
    std::string_view header(buffer, size * nitems);
    const auto total = nitems * size;
    
    if (header.find(':') == std::string::npos) {
        return total;
    }

    std::vector<std::string> parts;
    boost::split(parts, header, boost::is_any_of(":"));
    if (parts.size() != 2) {
        return total;
    }

    resp->headers[std::string(parts[0])] = std::string(parts[1]);
    return total;
}

HttpRequest::HttpRequest(const std::string& uri, const Headers& headers, bool allowSelfSigned) {
    _curl = curl_easy_init();
    LOG_INFO("CURL REQUEST TO: " << uri << std::endl);
    curl_easy_setopt(_curl, CURLOPT_URL, uri.c_str());

    for (const auto& [key, value] : headers) {
        std::ostringstream h;
        h << key << ": " << value;
        _headers = curl_slist_append(_headers, h.str().c_str());
    }

    curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _headers);

    if (allowSelfSigned) {
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0);
    }
}

HttpRequest::~HttpRequest() {
    curl_easy_cleanup(_curl);
    curl_slist_free_all(_headers);
}

HttpResponsePtr HttpRequest::execute() {
    auto resp = std::make_unique<HttpResponse>();

    std::ostringstream buffer;
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, curlStringStreamCallback);
    curl_easy_setopt(_curl, CURLOPT_HEADERDATA, resp.get());
    curl_easy_setopt(_curl, CURLOPT_HEADERFUNCTION, curlHeaderCallback);

    resp->curlError = curl_easy_perform(_curl);
    if (resp->curlError == CURLE_OK) {
        curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &resp->status);
    }

    resp->body = buffer.str();
    return resp;
}

void HttpRequest::doDelete() {
    curl_easy_setopt(_curl, CURLOPT_CUSTOMREQUEST, "DELETE");
}

void HttpRequest::doPost(const nlohmann::json& body) {
    curl_easy_setopt(_curl, CURLOPT_POST, 1);

    std::ostringstream h;
    h << "Content-Type: application/json";
    _headers = curl_slist_append(_headers, h.str().c_str());
    curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _headers);

    const std::string jsonBody = body.dump();
    curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, jsonBody.size());
    curl_easy_setopt(_curl, CURLOPT_COPYPOSTFIELDS, jsonBody.c_str());
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

    std::unique_lock<std::shared_mutex> guard(_headerMutex);
    _headers["Authorization"] = str.str();
}

void HttpClient::setHeaderKeyValue(const std::string& key, const std::string& value) {
    std::unique_lock<std::shared_mutex> guard(_headerMutex);
    _headers[key] = value;
}

HttpResponsePtr HttpClient::get(const std::string& path) const {
    return sendRequest(path, nullptr);
}

HttpResponsePtr HttpClient::post(const std::string& path, const nlohmann::json& body) const {
    return sendRequest(path, [&body](HttpRequest& req){
        req.doPost(body);
    });
}

HttpResponsePtr HttpClient::del(const std::string& path) const {
    return sendRequest(path, [](HttpRequest& req){
        req.doDelete();
    });
}

HttpResponsePtr HttpClient::sendRequest(const std::string& path, const MethodRequestCallback& cb) const {
    tickRateLimit();

    std::ostringstream fullPath;
    fullPath << _baseUri << path;

    std::unique_ptr<HttpRequest> req;
    {
        // Need to sepaarate out this so modifying the headers
        // map isn't blocked by how long the request takes to run.
        std::shared_lock<std::shared_mutex> guard(_headerMutex);
        req.reset(new HttpRequest(fullPath.str(), _headers, _allowSelfSigned));
    }

    if (!!cb) {
        cb(*req);
    }

    auto resp = req->execute();
    for (const auto& cb : _responseInterceptors) {
        if (!cb(*resp)) {
            break;
        }
    }
    return std::move(resp);
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