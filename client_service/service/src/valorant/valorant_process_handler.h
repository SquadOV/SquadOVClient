#pragma once

#include "process_watcher/handler.h"
#include <memory>

namespace service::valorant {

class ValorantProcessHandlerInstance;
class ValorantProcessHandler : public process_watcher::ProcessWatchHandler {
public:
    // These are needed for using the forward declared ValorantProcessHandlerInstance as a unique_ptr.
    ValorantProcessHandler();
    ~ValorantProcessHandler();

private:
    void onProcessStarts() override;
    void onProcessStops() override;

    std::unique_ptr<ValorantProcessHandlerInstance> _instance;
};

}