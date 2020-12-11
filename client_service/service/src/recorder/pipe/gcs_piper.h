#pragma once

#include "recorder/pipe/file_output_piper.h"
#include "http/http_client.h"

#include <deque>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

#define DUMP_GCS_REF_VIDEO 0
#if DUMP_GCS_REF_VIDEO
#include <fstream>
#endif

namespace service::recorder::pipe {

class GCSPacket {
public:
    GCSPacket(const char* data, size_t dataSize):
        _data(data, data + dataSize)
    {}

    size_t size() const { return _data.size(); }
    const char* data() const { return _data.data(); }

private:
    std::vector<char> _data;
};

class GCSPiper : public FileOutputPiper {
public:
    GCSPiper(const std::string& destination, PipePtr&& pipe);
    ~GCSPiper();

    bool handleBuffer(const char* buffer, size_t numBytes) override;
    std::string sessionId() const override { return _sessionUri; }
    void flush() override;
private:
    void tickGcsThread();

    void sendDataFromBufferToGcsWithBackoff(std::vector<char>& buffer, size_t maxBytes, bool isLast);
    size_t sendDataFromBufferToGcs(const char* buffer, size_t numBytes, bool isLast);
    void copyDataIntoInternalBuffer(std::vector<char>& buffer);

    // URL returned by GCS to upload to.
    std::string _sessionUri;
    size_t _uploadedBytes = 0;

    service::http::HttpClientPtr _httpClient;
    std::mutex _gcsMutex;
    std::deque<GCSPacket> _gcsBuffer;
    std::thread _gcsThread;
    bool _finished = false;

    // Random number generator for backoff
    std::random_device _rd;
    std::mt19937 _gen;

#if DUMP_GCS_REF_VIDEO
    std::ofstream _refVideo;
#endif 
};

}