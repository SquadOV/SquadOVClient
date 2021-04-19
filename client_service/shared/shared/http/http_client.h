#pragma once

#include "shared/time.h"

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <shared_mutex>
#include <unordered_map>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

struct curl_slist;

namespace shared::http {

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
using DownloadProgressFn = std::function<void(size_t, size_t)>;

class HttpRequest;
class HttpClient {
public:
    explicit HttpClient(const std::string& baseUri);
    ~HttpClient();

    // This will be the number of seconds we wait between executing http requests using this client.
    void setRateLimit(double secondsPerTask) { _secondsPerTask = secondsPerTask; }

    void setBearerAuthToken(const std::string& token);
    void setHeaderKeyValue(const std::string& key, const std::string& value);
    void removeHeaderKey(const std::string& key);
    void setMaxUploadBytesPerSec(size_t v) { _maxUploadSpeed = v; }

    void enableSelfSigned() { _allowSelfSigned = true; }
    void quiet() { _quiet = true; }

    void addResponseInterceptor(const ResponseInterceptor& i) { _responseInterceptors.push_back(i); }
    void clearResponseInterceptors() { _responseInterceptors.clear(); }

    void addDownloadProgressFn(const DownloadProgressFn& fn) { _downloadProgressCallbacks.push_back(fn); }

    void setTimeout(long v) { _timeoutSeconds = v; }
    void clearTimeout() { _timeoutSeconds = {}; }

    HttpResponsePtr download(const std::string& path, const std::filesystem::path& output) const;
    HttpResponsePtr get(const std::string& path) const;
    HttpResponsePtr post(const std::string& path, const nlohmann::json& body, bool forceGzip = false, bool quiet = false) const;
    HttpResponsePtr put(const std::string& path, const char* buffer, size_t numBytes) const;
    HttpResponsePtr del(const std::string& path) const;

private:
    using MethodRequestCallback = std::function<void(HttpRequest&)>;
    HttpResponsePtr sendRequest(const std::string& path, const MethodRequestCallback& cb, bool quiet) const;

    void tickRateLimit() const;
    bool _quiet = false;
    // TLS
    bool _allowSelfSigned = false;
    // Rate limiting
    double _secondsPerTask = 0.0;
    std::optional<size_t> _maxUploadSpeed;
    // Unix time
    mutable shared::TimePoint _lastRequestTime;
    mutable std::shared_mutex _rateLimitMutex;
    // Timeout
    std::optional<long> _timeoutSeconds;

    std::string _baseUri;

    mutable std::shared_mutex _headerMutex;
    Headers _headers;

    // Intercepts. Let users modify the response/request as necessary.
    std::vector<ResponseInterceptor> _responseInterceptors;
    std::vector<DownloadProgressFn> _downloadProgressCallbacks;
};

using HttpClientPtr = std::unique_ptr<HttpClient>;

}