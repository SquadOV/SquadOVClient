#include "recorder/pipe/pipe_client.h"
#include "shared/errors/error.h"


#include <sstream>

namespace service::recorder::pipe {

PipeClient::PipeClient(const std::string& name, std::vector<char> wBuffer) {

#ifdef _WIN32
    std::ostringstream pipePath;
    pipePath << "\\\\.\\pipe\\" << name;
    _filePath = pipePath.str();
    dwMode = PIPE_READMODE_BYTE; 

    connectToPipe();
    // The pipe connected; change to message-read mode. 

    handleState();
    // Send a message to the pipe server. 
    
    writeDataToPipe(wBuffer);

    return;
#else
    throw std::runtime_error("Unsupported OS for Named Pipes");
#endif
}

void PipeClient::writeDataToPipe(std::vector<char> wBuffer) {
    fSuccess = WriteFile(
        _hPipe,                  // pipe handle 
        (void*)wBuffer.data(),  // message 
        wBuffer.size(),         // message length 
        NULL,             // bytes written 
        NULL);                  // not overlapped 

    if ( ! fSuccess) 
    {
        _tprintf( TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError() ); 
        return;
    }

    printf("\nMessage sent to server, receiving reply as follows:\n");
 
}

void PipeClient::handleState() {
    fSuccess = SetNamedPipeHandleState( 
        _hPipe,    // pipe handle 
        &dwMode,  // new pipe mode 
        NULL,     // don't set maximum bytes 
        NULL);    // don't set maximum time 
    if ( ! fSuccess) 
    {
      _tprintf( TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError() ); 
      return;
    }
}

void PipeClient::connectToPipe() {
    while (1) {
        _hPipe = CreateFile(
            _filePath.c_str(), // lpFileName
            GENERIC_READ | GENERIC_WRITE, // dwDesiredAccess
            0, // dwShareMode
            NULL, // lpSecurityAttributes
            OPEN_EXISTING, // dwCreationDisposition
            0, // dwFlagsAndAttributes
            NULL // hTemplateFile
        );

    // Break if the pipe handle is valid. 
    
        if (_hPipe != INVALID_HANDLE_VALUE) 
            break;
    
        // Exit if an error other than ERROR_PIPE_BUSY occurs. 
    
        if (GetLastError() != ERROR_PIPE_BUSY) 
        {
            THROW_ERROR("Failed to create file: " << GetLastError());
            return;
        }
    
        // All pipe instances are busy, so wait for 20 seconds. 
    
        if ( ! WaitNamedPipe(_filePath.c_str(), 20000)) 
        { 
            printf("Could not open pipe: 20 second wait timed out."); 
            return;
        }
    }
}

PipeClient::~PipeClient() {
    CloseHandle(_hPipe);
}
};