#pragma once

#include "recorder/pipe/pipe.h"
#include <memory>
#include <string>
#include <thread>

namespace service::recorder::pipe {

// This class manages creating and reading from an OS pipe
// and outputting the contents of the pipe to some storage device.
class FileOutputPiper {
public:
    explicit FileOutputPiper(PipePtr&& pipe);
    virtual ~FileOutputPiper();

    void start();
    void wait();
    void stop();

    // At least on windows the pipe will have an actual path on disk that
    // we can use.
    const std::string& filePath() const { return _pipe->filePath(); }

protected:
    virtual bool handleBuffer(const char* buffer, size_t numBytes) = 0;
    virtual void flush() = 0;

private:
    bool _running = true;
    std::thread _pipeThread;
    PipePtr _pipe;
};

using FileOutputPiperPtr = std::unique_ptr<FileOutputPiper>;

FileOutputPiperPtr createFileOutputPiper(const std::string& id, const std::string& destination);

}