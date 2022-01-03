#include "shared/system/win32/process.h"
#include "shared/errors/error.h"
#include "shared/log/log.h"

namespace shared::system::win32 {

void elevateProcessPriority(DWORD priority) {
    HANDLE process = GetCurrentProcess();
    if (!SetPriorityClass(process, priority)) {
        LOG_ERROR("Failed to elevate process priority: " << shared::errors::getWin32ErrorAsString() << std::endl);
    }
}

}