#include "recorder/pipe/pipe.h"
#include "shared/errors/error.h"

#include <sstream>

namespace service::recorder::pipe {

Pipe::Pipe(const std::string& name) {
    // At least on windows the Pipe has a filepath of the form
    // \\.\pipe\NAME
#ifdef _WIN32
    std::ostringstream pipePath;
    pipePath << "\\\\.\\pipe\\" << name;
    _filePath = pipePath.str();

    _namedPipe = CreateNamedPipeA(
        _filePath.c_str(), // lpName
        PIPE_ACCESS_INBOUND, // dwOpenMode
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, // dwPipeMode
        1, // nMaxInstances
        0, // nOutBufferSize,
        MAX_BUFFER_SIZE, // nInBufferSize,
        0, // nDefaultTimeOut
        nullptr // lpSecurityAttributes
    );

    if (_namedPipe == INVALID_HANDLE_VALUE) {
        THROW_ERROR("Failed to create named pipe: " << GetLastError());
        return;
    }
#else
    throw std::runtime_error("Unsupported OS for Named Pipes");
#endif
}

Pipe::~Pipe() {
    DisconnectNamedPipe(_namedPipe);
    CloseHandle(_namedPipe);
}

void Pipe::waitForConnect() {
    const auto connected = ConnectNamedPipe(_namedPipe, nullptr);
    if (!connected && GetLastError() != ERROR_PIPE_CONNECTED) {
        THROW_ERROR("Failed to connect named pipe.");
    }
}

size_t Pipe::readFromBuffer() {
    size_t bytesRead = 0;
    const auto success = ReadFile(
        _namedPipe, // hFile
        (void*)_buffer.data(), // lpBuffer,
        MAX_BUFFER_SIZE, // nNumberOfBytesToRead,
        (DWORD*)&bytesRead, // lpNumberOfBytesRead
        nullptr
    );

    if (!success) {
        LOG_WARNING("Failed to read pipe buffer: " << GetLastError() << std::endl);
        return 0;
    }

    return bytesRead;
}

}