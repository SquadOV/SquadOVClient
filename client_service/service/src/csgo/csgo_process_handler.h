#pragma once

#include "process_watcher/handler.h"
#include "process_watcher/process/process.h"
#include <memory>

namespace service::csgo {

class CsgoProcessHandlerInstance;
class CsgoProcessHandler : public process_watcher::ProcessWatchHandler {
public:
    // These are needed for using the forward declared CsgoProcessHandlerInstance as a unique_ptr.
    CsgoProcessHandler();
    ~CsgoProcessHandler();

private:
    void onProcessStarts(const process_watcher::process::Process& p) override;
    void onProcessStops() override;

    std::unique_ptr<CsgoProcessHandlerInstance> _instance;
};

}