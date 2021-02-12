#pragma once
#include <memory>
#include "process_watcher/process/process.h"

namespace process_watcher {

class ProcessWatchHandler {
public:
    virtual ~ProcessWatchHandler() {}
    virtual void onProcessStarts(const process::Process& p) = 0;
    virtual void onProcessStops() = 0;
};

using ProcessWatchHandlerPtr = std::unique_ptr<ProcessWatchHandler>;

}