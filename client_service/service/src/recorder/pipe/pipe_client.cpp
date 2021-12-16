#include "recorder/pipe/pipe_client.h"
#include "shared/errors/error.h"


#include <sstream>

namespace service::recorder::pipe {

PipeClient::PipeClient(const std::string& name) {

#ifdef _WIN32
    std::ostringstream pipePath;
    pipePath << "\\\\.\\pipe\\" << name;
    _filePath = pipePath.str();

    // The pipe connected; change to message-read mode. 
    connectToPipe();

    // Send a message to the pipe server. 
    handleState();

#else
    throw std::runtime_error("Unsupported OS for Named Pipes");
#endif
}

void PipeClient::start(const std::vector<char>& wBuffer) {
    BOOL fSuccess = WriteFile(
        _hPipe,                  // pipe handle 
        (void*)wBuffer.data(),  // message 
        wBuffer.size(),         // message length 
        NULL,             // bytes written 
        NULL);                  // not overlapped 

    if ( !fSuccess ) 
    {
        THROW_ERROR("WriteFile to pipe failed: "<< GetLastError() ); 
        return;
    }
}

void PipeClient::stop() {
    if(_hPipe) {
        CloseHandle(_hPipe);
        _hPipe = NULL;
    }
}

void PipeClient::handleState() {
    BOOL fSuccess = SetNamedPipeHandleState( 
        _hPipe,    // pipe handle 
        PIPE_READMODE_BYTE,  // new pipe mode 
        NULL,     // don't set maximum bytes 
        NULL);    // don't set maximum time 
    if ( !fSuccess ) {
      THROW_ERROR("SetNamedPipeHandleState failed: " << GetLastError());
    }
}

void PipeClient::connectToPipe() {
    for ( int i =0; i < 5; i++ ) {
        _hPipe = CreateFile(
            _filePath.c_str(), // lpFileName
            GENERIC_WRITE, // dwDesiredAccess
            0, // dwShareMode
            NULL, // lpSecurityAttributes
            OPEN_EXISTING, // dwCreationDisposition
            0, // dwFlagsAndAttributes
            NULL // hTemplateFile
        );

    // Break if the pipe handle is valid. 
    
        if (_hPipe != INVALID_HANDLE_VALUE) {
            break;
        }
    
        // Exit if an error other than ERROR_PIPE_BUSY occurs. 
    
        if (GetLastError() != ERROR_PIPE_BUSY) {
            THROW_ERROR("Failed to create file: " << GetLastError());
            return;
        }
    
        // All pipe instances are busy, so wait for 5 second. 
        if ( ! WaitNamedPipe(_filePath.c_str(), 1000)) { 
            LOG_WARNING("Could not open pipe: 5 second wait timed out."); 
            return;
        }

        if ( i == 4 ) {
            LOG_WARNING("Unable to Connect to Pipe after 5 tries.")
        }
    }
}

PipeClient::~PipeClient() {
    if(_hPipe) {
        CloseHandle(_hPipe);
        _hPipe = NULL;
    }
}
};