#pragma once

#include "process_watcher/handler.h"
#include "process_watcher/process/process.h"
#include <memory>

namespace service::wow {

class WoWProcessHandlerInstance;
class WoWProcessHandler : public process_watcher::ProcessWatchHandler {
public:
    WoWProcessHandler();
    ~WoWProcessHandler();

private:
    void onProcessStarts(const process_watcher::process::Process& p) override;
    void onProcessStops() override;

    std::unique_ptr<WoWProcessHandlerInstance> _instance;
};

}