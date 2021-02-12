#include "recorder/pipe/file_output_piper.h"

#include "shared/errors/error.h"
#include "recorder/pipe/filesystem_piper.h"
#include "recorder/pipe/gcs_piper.h"

#include <boost/algorithm/string.hpp>
#include <chrono>

namespace service::recorder::pipe {

FileOutputPiper::FileOutputPiper(PipePtr&& pipe):
    _pipe(std::move(pipe)) {
}

FileOutputPiper::~FileOutputPiper() {

}

void FileOutputPiper::start() {
    _pipeThread = std::thread([this](){
        _pipe->waitForConnect();
        LOG_INFO("Successfully connected to pipe..." << std::endl);

        const auto* buffer = _pipe->buffer();
        while (_running) {
            std::unique_lock<std::mutex> lock(_pauseMutex);
            if (!_pauseCv.wait_for(lock, std::chrono::milliseconds(100), [this](){ return !_paused; })) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            const auto numBytes = _pipe->readFromBuffer();

            if (!handleBuffer(buffer, numBytes)) {
                LOG_WARNING("Failed to handle buffer of size: " << numBytes << std::endl);
                break;
            }

            // This break needs to be behind the handleBuffer call because
            // we still want to handle 0 bytes as that'll be the indication
            // that the recording stopped so we want to write out any last
            // information (or to do any finishing steps).
            if (numBytes == 0) {
                break;
            }
        }

        flush();
    });
}

void FileOutputPiper::stop() {
    _running = false;
}

void FileOutputPiper::wait() {
    if (_pipeThread.joinable()) {
        _pipeThread.join();
    }
}

FileOutputPiperPtr createFileOutputPiper(const std::string& id, const std::string& destination) {
    auto pipe = std::make_unique<Pipe>(id);
    if (boost::starts_with(destination, "https://storage.googleapis.com")) {
        return std::make_unique<GCSPiper>(destination, std::move(pipe));
    } else {
        return std::make_unique<FilesystemPiper>(destination, std::move(pipe));
    }
}

void FileOutputPiper::appendFromFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    std::array<char, 2048> buffer;
    while (!input.eof()) {
        input.read(buffer.data(), buffer.size());
        const auto read = input.gcount();
        handleBuffer(buffer.data(), static_cast<size_t>(read));
    }
}

void FileOutputPiper::pauseProcessingFromPipe(bool pause) {
    {
        std::lock_guard<std::mutex> lock(_pauseMutex);
        _paused = pause;
    }

    if (!_paused) {
        _pauseCv.notify_all();
    }
}

}