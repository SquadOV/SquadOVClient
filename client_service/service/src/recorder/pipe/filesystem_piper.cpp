#include "recorder/pipe/filesystem_piper.h"


namespace fs = std::filesystem;

namespace service::recorder::pipe {

FilesystemPiper::FilesystemPiper(const std::string& destination, PipePtr&& pipe):
    FileOutputPiper(std::move(pipe)),
    _outputFile(destination) {
    const auto parentPath = _outputFile.parent_path();
    if (!fs::exists(parentPath)) {
        fs::create_directories(parentPath);
    }
    
    _output.open(_outputFile, std::ios_base::binary | std::ios_base::out);
}

FilesystemPiper::~FilesystemPiper() {
    _output.close();
}

bool FilesystemPiper::handleBuffer(const char* buffer, size_t numBytes) {
    _output.write(buffer, numBytes);
    return true;
}

}