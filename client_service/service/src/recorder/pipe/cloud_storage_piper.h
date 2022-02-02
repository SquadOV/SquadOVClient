#pragma once

#include "recorder/pipe/file_output_piper.h"
#include "shared/http/http_client.h"
#include "recorder/pipe/cloud/cloud_storage_client.h"
#include "uploader/uploader.h"

#include <deque>
#include <mutex>
#include <nlohmann/json.hpp>
#include <random>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <chrono>

#define DUMP_CLOUD_REF_VIDEO 0
#if DUMP_CLOUD_REF_VIDEO
#include <fstream>
#endif

namespace service::recorder::pipe {

struct CloudUploadRequest {
    std::string task;
    std::string file;
    service::uploader::UploadDestination destination;

    static CloudUploadRequest fromJson(const nlohmann::json& json);
};

// (session, parts)
std::pair<std::string, std::vector<std::string>> uploadToCloud(const CloudUploadRequest& req, const shared::http::DownloadUploadProgressFn& progressFn);

class CloudStoragePacket {
public:
    CloudStoragePacket(const char* data, size_t dataSize):
        _data(data, data + dataSize)
    {}

    size_t size() const { return _data.size(); }
    const char* data() const { return _data.data(); }

private:
    std::vector<char> _data;
};

class CloudStoragePiper : public FileOutputPiper {
public:
    CloudStoragePiper(const std::string& videoUuid, const service::uploader::UploadDestination& destination, PipePtr&& pipe, bool allowAcceleration);
    ~CloudStoragePiper();

    std::string sessionId() const override { return _destination.session; }
    void setProgressCallback(const shared::http::DownloadUploadProgressFn& progressFn, size_t totalBytes);
    void setCurlProgressCallback(const shared::http::DownloadUploadProgressFn& progressFn);
    const std::vector<std::string>& segmentIds() const override { return _allSegmentsIds; };
    size_t getUploadedBytes() { return _uploadedBytes; };
    std::chrono::milliseconds getMillisecondsSpentUploading() { return std::chrono::duration_cast<std::chrono::milliseconds>(_lastUploadTime-_timeStart); };
    void skipFlush() { _skipFlush = true; };
    void flush() override;
    
protected:
    bool handleBuffer(const char* buffer, size_t numBytes) override;

private:
    void tickUploadThread();

    void sendDataFromBufferWithBackoff(std::vector<char>& buffer, size_t maxBytes, bool isLast);
    void copyDataIntoInternalBuffer(std::vector<char>& buffer);

    std::string _videoUuid;
    service::uploader::UploadDestination _destination;
    size_t _uploadedBytes = 0;

    cloud::CloudStorageClientPtr _client;
    std::mutex _cloudMutex;
    std::deque<CloudStoragePacket> _cloudBuffer;
    std::thread _cloudThread;
    std::vector<std::string> _allSegmentsIds;
    bool _finished = false;
    bool _skipFlush = false;
    shared::TimePoint _timeStart;
    shared::TimePoint _lastUploadTime;

    // Random number generator for backoff
    std::random_device _rd;
    std::mt19937 _gen;

#if DUMP_CLOUD_REF_VIDEO
    std::ofstream _refVideo;
#endif 

    std::optional<shared::http::DownloadUploadProgressFn> _progressFn;
    std::optional<size_t> _totalUploadBytes;
};

}