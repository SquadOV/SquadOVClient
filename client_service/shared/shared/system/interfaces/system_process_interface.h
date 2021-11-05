#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
using OSPID = DWORD;
using OSString = std::wstring;
using OSWindow = HWND;
#endif

namespace shared::system {

class SystemProcessInterface {
public:
    virtual ~SystemProcessInterface() {}

    virtual std::vector<OSPID> enumProcesses() const = 0;
    virtual OSString getProcessName(OSPID pid) const = 0;
    virtual OSWindow findWindowForProcessWithMaxDelay(OSPID pid, const std::chrono::milliseconds& maxDelayMs = std::chrono::milliseconds(0), const std::chrono::milliseconds& step = std::chrono::milliseconds(1000), bool quiet = false, bool checkWindowSize = true) const = 0;
};
using SystemProcessInterfacePtr = std::shared_ptr<SystemProcessInterface>;

}