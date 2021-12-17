#pragma once

#include "recorder/process_record_interface.h"
#include "process_watcher/handler.h"
#include "process_watcher/process/process.h"
#include <memory>

namespace service::database {
class DatabaseApi;
}

namespace service::valorant {

class ValorantProcessHandlerInstance;
class ValorantProcessHandler : public process_watcher::ProcessWatchHandler, public recorder::ProcessRecordInterface {
public:
    // These are needed for using the forward declared ValorantProcessHandlerInstance as a unique_ptr.
    ValorantProcessHandler();
    ~ValorantProcessHandler();

    void forceStopRecording() override;
private:
    void onProcessStarts(const process_watcher::process::Process& p) override;
    void onProcessStops() override;

    std::unique_ptr<ValorantProcessHandlerInstance> _instance;
};

}