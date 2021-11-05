#pragma once

#include "shared/system/interfaces/system_process_interface.h"

namespace shared::system::win32::interfaces {

class Win32SystemProcessInterface: public SystemProcessInterface {
public:
    std::vector<OSPID> enumProcesses() const override;
    OSString getProcessName(OSPID pid) const override;
    OSWindow findWindowForProcessWithMaxDelay(OSPID pid, const std::chrono::milliseconds& maxDelayMs = std::chrono::milliseconds(0), const std::chrono::milliseconds& step = std::chrono::milliseconds(1000), bool quiet = false, bool checkWindowSize = true) const override;
};

}