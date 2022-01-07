#include "shared/http/http_client.h"

#include "shared/constants.h"
#include "shared/log/log.h"
#include "shared/filesystem/utility.h"
#include "shared/http/dns_manager.h"

#include <boost/algorithm/string.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <curl/curl.h>
#include <optional>
#include <iostream>
#include <sstream>
#include <thread>

namespace fs = std::filesystem;
namespace shared::http {
namespace {

constexpr size_t MAX_RAW_JSON_POST_BYTES_SIZE = 1 * 1024 * 1024;
constexpr int MAX_RETRY = 3;
constexpr int MAX_RETRY_BACKOFF = 15000;

}

struct CurlRawBuffer {
    const char* buffer = nullptr;
    size_t size = 0;
    size_t read = 0;

    const char* currentPtr() const { return buffer + read; }
    size_t bufferSize(size_t req) const {
        return std::min(req, size - read);
    }
    void step(size_t bytes) {
        read += bytes;
    }
};

class HttpRequest {
public:
    HttpRequest(const std::string& uri, const Headers& headers, bool allowSelfSigned, bool quiet);
    ~HttpRequest();

    void setTimeout(long timeoutSeconds);
    void setUploadSpeed(size_t bytesPerSec);
    void addProgressCallbacks(const DownloadUploadProgressFn& fn) { _downloadUploadProgressCallbacks.push_back(fn); }

    HttpResponsePtr execute();

    void doDelete();
    void doPost(const nlohmann::json& body, bool forceGzip);
    void doPost(const std::vector<char>& body, bool forceGzip);
    void doPut(const char* buffer, size_t numBytes);

    void progressCallback(curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
    void outputToFile(const fs::path& output) { _outputFile = output; }
private:
    void setJsonBody(const nlohmann::json& body, bool forceGzip);
    void setRawPostBody(const char* body, size_t numBytes, const std::string& contentType, bool forceGzip);

    void setRawBuffer(const char* buffer, size_t size);

    CURL* _curl = nullptr;
    CurlRawBuffer _buffer;
    curl_slist* _headers = nullptr;
    char _errBuffer[CURL_ERROR_SIZE];
    std::vector<DownloadUploadProgressFn> _downloadUploadProgressCallbacks;
    std::optional<fs::path> _outputFile;
};

size_t curlReadCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    CurlRawBuffer* rawBuffer = reinterpret_cast<CurlRawBuffer*>(userdata);
    const auto* bytes = rawBuffer->currentPtr();
    const auto bytesHandled = rawBuffer->bufferSize(size * nitems);
    std::memcpy(buffer, bytes, bytesHandled);
    rawBuffer->step(bytesHandled);
    return bytesHandled;
}

size_t curlStringStreamCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::ostringstream& buffer = *reinterpret_cast<std::ostringstream*>(userdata);
    buffer << std::string(ptr, nmemb);
    return nmemb;
}

size_t curlOutputFileCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::ofstream& buffer = *reinterpret_cast<std::ofstream*>(userdata);
    buffer.write(ptr, nmemb);
    return nmemb;
}

size_t curlHeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    HttpResponse* resp = reinterpret_cast<HttpResponse*>(userdata);
    std::string_view header(buffer, size * nitems);
    const auto total = nitems * size;
    
    const auto splitIt = header.find(":");
    if (splitIt == std::string::npos) {
        return total;
    }

    // Don't do a boost::split here on a colon because a header value could have
    // more than one colon like the "Location" header...yikes...
    const std::string key = boost::algorithm::trim_copy(std::string(header.substr(0, splitIt)));
    const std::string value = boost::algorithm::trim_copy(std::string(header.substr(splitIt + 2)));
    resp->headers[key] = value;
    return total;
}

int curlProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    HttpRequest* req = reinterpret_cast<HttpRequest*>(clientp);
    req->progressCallback(dltotal, dlnow, ultotal, ulnow);
    return CURL_PROGRESSFUNC_CONTINUE;
}

HttpRequest::HttpRequest(const std::string& uri, const Headers& headers, bool allowSelfSigned, bool quiet) {
    _curl = curl_easy_init();

    if (!quiet) {
        LOG_INFO("CURL REQUEST TO: " << uri << std::endl);
    }
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

    // Time in seconds CURL will try to connect to the server for.
    curl_easy_setopt(_curl, CURLOPT_CONNECTTIMEOUT, 30L);

    // Prefer IPv4 just in case there's issues with connecting to our servers via IPv6.
    curl_easy_setopt(_curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
}

void HttpRequest::setTimeout(long timeoutSeconds) {
    // Number of seconds CURL will wait for the transfer to complete.
    curl_easy_setopt(_curl, CURLOPT_TIMEOUT, timeoutSeconds);
}

void HttpRequest::setUploadSpeed(size_t bytesPerSec) {
    curl_easy_setopt(_curl, CURLOPT_MAX_SEND_SPEED_LARGE, static_cast<curl_off_t>(bytesPerSec));
}

void HttpRequest::progressCallback(curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    for (const auto& cb : _downloadUploadProgressCallbacks) {
        cb(static_cast<size_t>(dltotal), static_cast<size_t>(dlnow), static_cast<size_t>(ultotal), static_cast<size_t>(ulnow));
    }
}

HttpRequest::~HttpRequest() {
    curl_easy_cleanup(_curl);
    curl_slist_free_all(_headers);
}

HttpResponsePtr HttpRequest::execute() {
    auto resp = std::make_unique<HttpResponse>();

    std::ostringstream buffer;
    std::ofstream ofile;

    if (_outputFile.has_value()) {
        ofile.open(shared::filesystem::pathUtf8(_outputFile.value()), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
        curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &ofile);
        curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, curlOutputFileCallback);
    } else {
        curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, curlStringStreamCallback);
    }
    curl_easy_setopt(_curl, CURLOPT_HEADERDATA, resp.get());
    curl_easy_setopt(_curl, CURLOPT_HEADERFUNCTION, curlHeaderCallback);
    curl_easy_setopt(_curl, CURLOPT_XFERINFODATA, this);
    curl_easy_setopt(_curl, CURLOPT_XFERINFOFUNCTION, curlProgressCallback);
    if (!_downloadUploadProgressCallbacks.empty()) {
        curl_easy_setopt(_curl, CURLOPT_NOPROGRESS, 0);
    }

    _errBuffer[0] = 0;
    curl_easy_setopt(_curl, CURLOPT_ERRORBUFFER, _errBuffer);

    for (int i = 0; i < MAX_RETRY; ++i) {
        resp->curlError = curl_easy_perform(_curl);
        if (resp->curlError == CURLE_OK) {
            curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &resp->status);
            resp->body = buffer.str();
            break;
        } else {
            resp->body = std::string(_errBuffer);
            LOG_WARNING("Generic CURL failure: " << resp->body << std::endl);

            if (i < MAX_RETRY - 1) {
                // Exponential backoff with jitter algorithm.
                // See: https://cloud.google.com/iot/docs/how-tos/exponential-backoff
                // Some notable changes are that we have a base minimum of a 100ms backoff.
                const auto backoffMs = std::min(static_cast<int>(100 + std::pow(2, i + 1) + rand() / RAND_MAX * 1000), MAX_RETRY_BACKOFF);
                std::this_thread::sleep_for(std::chrono::milliseconds(backoffMs));
            }
        }
    }

    if (ofile.is_open()) {
        ofile.close();
    }
    
    return resp;
}

void HttpRequest::doDelete() {
    curl_easy_setopt(_curl, CURLOPT_CUSTOMREQUEST, "DELETE");
}

void HttpRequest::setJsonBody(const nlohmann::json& body, bool forceGzip) {
    const std::string jsonBody = body.dump();
    setRawPostBody(jsonBody.c_str(), jsonBody.size(), "application/json", forceGzip);
}

void HttpRequest::setRawPostBody(const char* body, size_t numBytes, const std::string& contentType, bool forceGzip) {
    if (numBytes == 0) {
        curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, 0);
    } else {
        std::ostringstream contentTypeHeader;
        contentTypeHeader << "Content-Type: " << contentType;
        _headers = curl_slist_append(_headers, contentTypeHeader.str().c_str());

        if (forceGzip || numBytes > MAX_RAW_JSON_POST_BYTES_SIZE) {
            _headers = curl_slist_append(_headers, "Content-Encoding: gzip");

            std::vector<char> gzipBuffer;
            auto gzipDevice = boost::iostreams::back_inserter(gzipBuffer);
            boost::iostreams::filtering_ostream outputStream;
            outputStream.push(boost::iostreams::gzip_compressor{});
            outputStream.push(gzipDevice);
            outputStream << std::string_view(body, numBytes) << std::flush;
            outputStream.pop();

            curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, gzipBuffer.size());
            curl_easy_setopt(_curl, CURLOPT_COPYPOSTFIELDS, gzipBuffer.data());
        } else {
            curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, numBytes);
            curl_easy_setopt(_curl, CURLOPT_COPYPOSTFIELDS, body);
        }
        curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _headers);
    }
}

void HttpRequest::doPost(const nlohmann::json& body, bool forceGzip) {
    curl_easy_setopt(_curl, CURLOPT_POST, 1);
    setJsonBody(body, forceGzip);
}

void HttpRequest::doPost(const std::vector<char>& body, bool forceGzip) {
    curl_easy_setopt(_curl, CURLOPT_POST, 1);
    setRawPostBody(body.data(), body.size(), "application/octet-stream", forceGzip);
}

void HttpRequest::setRawBuffer(const char* buffer, size_t size) {
    if (size == 0) {
        return;
    }

    _buffer.buffer = buffer;
    _buffer.size = size;
    _buffer.read = 0;

    curl_easy_setopt(_curl, CURLOPT_READDATA, &_buffer);
    curl_easy_setopt(_curl, CURLOPT_READFUNCTION, curlReadCallback);
    curl_easy_setopt(_curl, CURLOPT_INFILESIZE, size);
}

void HttpRequest::doPut(const char* buffer, size_t numBytes) {
    curl_easy_setopt(_curl, CURLOPT_UPLOAD, 1);
    setRawBuffer(buffer, numBytes);
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

void HttpClient::removeHeaderKey(const std::string& key) {
    std::unique_lock<std::shared_mutex> guard(_headerMutex);
    _headers.erase(key);
}

HttpResponsePtr HttpClient::download(const std::string& path, const std::filesystem::path& output) const {
    return sendRequest(path, [&output](HttpRequest& req){
        req.outputToFile(output);
    });
}

HttpResponsePtr HttpClient::get(const std::string& path) const {
    return sendRequest(path, nullptr);
}

HttpResponsePtr HttpClient::post(const std::string& path, const nlohmann::json& body, bool forceGzip) const {
    return sendRequest(path, [&body, forceGzip](HttpRequest& req){
        req.doPost(body, forceGzip);
    });
}

HttpResponsePtr HttpClient::post(const std::string& path, const std::vector<char>& buffer, bool forceGzip) const {
    return sendRequest(path, [&buffer, forceGzip](HttpRequest& req){
        req.doPost(buffer, forceGzip);
    });
}

HttpResponsePtr HttpClient::put(const std::string& path, const char* buffer, size_t numBytes) const {
    return sendRequest(path, [buffer, numBytes](HttpRequest& req){
        req.doPut(buffer, numBytes);
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
        req.reset(new HttpRequest(fullPath.str(), _headers, _allowSelfSigned, _quiet));
    }

    if (_timeoutSeconds.has_value()) {
        req->setTimeout(_timeoutSeconds.value());
    }

    for (const auto& cb: _downloadUploadProgressCallbacks) {
        req->addProgressCallbacks(cb);
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