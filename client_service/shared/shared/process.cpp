#include "shared/process.h"
#include "shared/errors/error.h"
#include "shared/filesystem/utility.h"
#include <Windows.h>

namespace shared::process {

int runProcessWithTimeout(const std::filesystem::path& exe, std::string commandLine, const std::optional<std::chrono::seconds>& timeoutSeconds) {
    STARTUPINFO info = {};
    info.cb = sizeof(STARTUPINFO);

    PROCESS_INFORMATION process = {};

    if (!CreateProcessA(
        shared::filesystem::pathUtf8(exe).c_str(),
        commandLine.data(),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &info,
        &process
    )) {
        THROW_ERROR("Failed to create process [" << exe << "\t" << commandLine << "]: " << shared::errors::getWin32ErrorAsString());
    }

    if (timeoutSeconds) {
        WaitForSingleObject(process.hProcess, timeoutSeconds.value().count() * 1000);
    } else {
        WaitForSingleObject(process.hProcess, INFINITE);
    }
    
    CloseHandle(process.hProcess);
    CloseHandle(process.hThread);

    DWORD code = 0;
    if (GetExitCodeProcess(process.hProcess, &code) || code == STILL_ACTIVE) {
        LOG_WARNING("Either we failed to get exit code or the process is still running. Killing it!" << std::endl);
        TerminateProcess(process.hProcess, 1);
        code = 1;
    }
    return code;
}

}