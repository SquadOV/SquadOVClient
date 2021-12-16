#pragma once

#include <string>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

constexpr int MAX_BUFFER_SIZE = 16 * 1024 * 1024;

namespace service::recorder::pipe {

class PipeClient {
public:
    explicit PipeClient(const std::string& name);
    ~PipeClient();
    void start(const std::vector<char>& wBuffer);
    void stop();
private:
    void connectToPipe();
    void handleState();
    std::string _filePath;

#ifdef _WIN32
    HANDLE _hPipe;
#endif
};
}