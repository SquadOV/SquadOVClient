#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#define BUFSIZE 512

#ifdef _WIN32
#include <Windows.h>
#endif

namespace service::recorder::pipe {

class PipeClient {
public:
    explicit PipeClient(const std::string& name, std::vector<char> wBuffer);
    ~PipeClient();

private:
    void connectToPipe();
    void handleState();
    void writeDataToPipe(std::vector<char> wBuffer);
    std::string _filePath;

#ifdef _WIN32
    HANDLE _hPipe;
    DWORD cbRead, cbToWrite, cbWritten, dwMode;
    BOOL fSuccess = FALSE;
    TCHAR chBuf[BUFSIZE];
    LPTSTR lpvMessage=TEXT("Default message from client.");
#endif
};
}