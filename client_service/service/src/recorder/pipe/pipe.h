#pragma once

#include <array>
#include <memory>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace service::recorder::pipe {
// 16MB buffer should be plenty? Keeping it on the higher side just in case
// we need to buffer a lot for the DVR functionality.
constexpr int MAX_BUFFER_SIZE = 16 * 1024 * 1024;

// This class wraps functionality around whatever the OS's named pipe
// functionality is. Note that this as the pipe's server and FFmpeg acts as the client.
// This is necessary for the FFmpeg encoding stuff because we need to get
// the data from FFmpeg onto either disk or GCS. If we *only* had to support
// IO to disk then we wouldn't need this as FFmpeg could handle that by itself.
// However we *also* need to support writing to Google Cloud Storage. How that'll work
// is that we'll need to do a streaming transfer via a resumable upload using
// a Google Cloud Storage signed URL. This difference in functionality is abstracted away by
// the FileOutputPiper base class. This class mainly just deals with
//  1) Creating the pipe
//  2) Knowing when the pipe gets written to
//  3) Reading from the pipe.
class Pipe {
public:
    explicit Pipe(const std::string& name);
    ~Pipe();

    const std::string& filePath() const { return _filePath; }
    void waitForConnect();
    size_t readFromBuffer();
    const char* buffer() const { return _buffer.data(); }

private:
    std::string _filePath;

    std::array<char, MAX_BUFFER_SIZE> _buffer;
#ifdef _WIN32
    HANDLE _namedPipe;
#endif
};

using PipePtr = std::unique_ptr<Pipe>;

}