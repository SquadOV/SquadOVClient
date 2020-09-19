#include "http/http_client.h"

#include <curl/curl.h>
#include <iostream>
#include <sstream>

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
    std::ostringstream fullPath;
    fullPath << _baseUri << path;

    HttpRequest req(fullPath.str(), _headers);
    return req.Do();
}

}