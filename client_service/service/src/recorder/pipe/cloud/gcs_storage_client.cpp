#include "recorder/pipe/cloud/gcs_storage_client.h"
#include "shared/errors/error.h"
#include <boost/algorithm/string.hpp>
namespace service::recorder::pipe::cloud {

GCSStorageClient::GCSStorageClient() {
    _httpClient = std::make_unique<shared::http::HttpClient>("");
    _httpClient->setHeaderKeyValue("content-type", "application/octet-stream");
}

void GCSStorageClient::initializeDestination(const service::uploader::UploadDestination& destination) {
    _destination = destination;
}

void GCSStorageClient::setProgressCallback(const shared::http::DownloadUploadProgressFn& progressFn) {
    _httpClient->addDownloadProgressFn(progressFn);
}

void GCSStorageClient::startNewSegment() {
}

std::pair<std::string, size_t> GCSStorageClient::uploadBytes(const char* buffer, size_t numBytes, bool isLast, size_t uploadedBytes) {
    _httpClient->setHeaderKeyValue("Content-Length", std::to_string(numBytes));

    std::ostringstream range;
    range << "bytes " << uploadedBytes << "-" << (uploadedBytes + numBytes - 1) << "/";
    if (isLast) {
        range << (uploadedBytes + numBytes);
    } else {
        range << "*";
    }
    _httpClient->setHeaderKeyValue("Content-Range", range.str());

    std::string retRange;
    _httpClient->addResponseInterceptor([&retRange](shared::http::HttpResponse& response){
        for (const auto& [key, value] : response.headers) {
            if (key == "Range") {
                retRange = value.substr(value.find("bytes=") + 6);
                break;
            }
        }
        return true;
    });

    const auto ret = _httpClient->put(_destination.url, buffer, numBytes);
    if (ret->status == 200 || ret->status == 201) {
        // 200/201 indicate that the upload was completed.
        return std::make_pair("", numBytes);
    } else if (ret->status == 308) {
        // 308 indicates that we need to continue uploading - we should use the Range header (retRange)
        // to determine how many bytes were actually sent. If no range header was returned then no
        // data has been uploaded.
        if (retRange.empty()) {
            return std::make_pair("", 0);
        }

        std::vector<std::string> rangeParts;
        boost::split(rangeParts, retRange, boost::is_any_of("-"));
        if (rangeParts.size() == 2) {
            // Use our own start here because GCS will return 0-X
            // to indicate *all* the data we've uploaded so far not just
            // this request.
            const auto start = uploadedBytes;
            const auto end = static_cast<size_t>(std::stoull(rangeParts[1]));
            return std::make_pair("", end - start + 1);
        } else {
            THROW_ERROR("Invalid range header received: " << retRange << std::endl);
        }

    } else {
        THROW_ERROR("Failed to send data to GCS: " << ret->status << "\t" << ret->body << std::endl);
    }
    
    return std::make_pair("", 0);
}

}