#pragma once

#include <memory>
#include <string>
#include <unordered_map>
struct curl_slist;

namespace service::http {

struct HttpResponse {
    std::string body;
    long status = 0;
    int curlError = 0;
};
using HttpResponsePtr = std::unique_ptr<HttpResponse>;
using Headers = std::unordered_map<std::string, std::string>;

class HttpClient {
public:
    explicit HttpClient(const std::string& baseUri);
    ~HttpClient();

    void setBearerAuthToken(const std::string& token);
    void setHeaderKeyValue(const std::string& key, const std::string& value);

    HttpResponsePtr Get(const std::string& path) const;

private:
    std::string _baseUri;
    Headers _headers;
};

}