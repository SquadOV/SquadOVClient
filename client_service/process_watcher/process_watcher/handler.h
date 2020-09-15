#pragma once
#include <memory>

namespace process_watcher {

class ProcessWatchHandler {
public:
    virtual void onProcessStarts() = 0;
    virtual void onProcessStops() = 0;
};

using ProcessWatchHandlerPtr = std::unique_ptr<ProcessWatchHandler>;

}