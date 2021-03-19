#pragma once
#include <memory>
#include "process_watcher/process/process.h"

namespace process_watcher {

class ProcessWatchHandler {
public:
    virtual ~ProcessWatchHandler() {}
    
    void publicOnProcessStarts(const process::Process& p) {
        onProcessStarts(p);
        _currentProcess = p;
    }

    virtual void onProcessStops() = 0;

    const process::Process& currentProcess() const { return _currentProcess; }

private:
    virtual void onProcessStarts(const process::Process& p) = 0;
    process::Process _currentProcess;
};

using ProcessWatchHandlerPtr = std::unique_ptr<ProcessWatchHandler>;

}