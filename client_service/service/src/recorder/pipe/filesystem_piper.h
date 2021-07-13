#pragma once

#include "recorder/pipe/file_output_piper.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace service::recorder::pipe {

class FilesystemPiper : public FileOutputPiper {
public:
    FilesystemPiper(const std::string& destination, PipePtr&& pipe);
    ~FilesystemPiper();
    void flush() override {}

    std::optional<std::filesystem::path> localFile() const override { return _outputFile; }
    const std::vector<std::string>& segmentIds() const override { return _segmentIds; };
    
protected:
    bool handleBuffer(const char* buffer, size_t numBytes) override;
private:
    std::filesystem::path _outputFile;
    std::vector<std::string> _segmentIds;
    std::ofstream _output;
};

}