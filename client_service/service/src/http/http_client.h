#pragma once

#include "shared/time.h"

#include <functional>
#include <memory>
#include <string>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

struct curl_slist;

namespace service::http {

using Headers = std::unordered_map<std::string, std::string>;
struct HttpResponse {
    Headers headers;
    std::string body;
    long status = 0;
    int curlError = 0;
};
using HttpResponsePtr = std::unique_ptr<HttpResponse>;

// Returns true if the next interceptor should still be run.
using ResponseInterceptor = std::function<bool(HttpResponse&)>;

class HttpClient {
public:
    explicit HttpClient(const std::string& baseUri);
    ~HttpClient();

    // This will be the number of seconds we wait between executing http requests using this client.
    void setRateLimit(double secondsPerTask) { _secondsPerTask = secondsPerTask; }

    void setBearerAuthToken(const std::string& token);
    void setHeaderKeyValue(const std::string& key, const std::string& value);
    void enableSelfSigned() { _allowSelfSigned = true; }

    void addResponseInterceptor(const ResponseInterceptor& i) { _responseInterceptors.push_back(i); }

    HttpResponsePtr Get(const std::string& path) const;

private:
    void tickRateLimit() const;
    // TLS
    bool _allowSelfSigned = false;
    // Rate limiting
    double _secondsPerTask = 0.0;
    // Unix time
    mutable shared::TimePoint _lastRequestTime;
    mutable std::shared_mutex _rateLimitMutex;

    std::string _baseUri;

    mutable std::shared_mutex _headerMutex;
    Headers _headers;

    // Intercepts. Let users modify the response/request as necessary.
    std::vector<ResponseInterceptor> _responseInterceptors;
};

}