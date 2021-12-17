#pragma once

#include <string>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

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