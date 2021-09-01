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

    void publicOnProcessStops() {
        onProcessStops();
        _currentProcess = process::Process();
    }

    // Return true/false depending on whether or not this mode is supported.
    // If it's not supported, then we should just restart the handler (stops -> starts).
    virtual bool onProcessChange(const process::Process& p) { return false; }

    const process::Process& currentProcess() const { return _currentProcess; }

private:
    virtual void onProcessStarts(const process::Process& p) = 0;
    virtual void onProcessStops() = 0;

    process::Process _currentProcess;
};

using ProcessWatchHandlerPtr = std::unique_ptr<ProcessWatchHandler>;

}