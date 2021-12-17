#pragma once

#include "recorder/process_record_interface.h"
#include "process_watcher/handler.h"
#include "process_watcher/process/process.h"
#include <memory>

namespace service::database {
class DatabaseApi;
}

namespace service::hearthstone {

class HearthstoneProcessHandlerInstance;
class HearthstoneProcessHandler : public process_watcher::ProcessWatchHandler, public recorder::ProcessRecordInterface {
public:
    // These are needed for using the forward declared HearthstoneProcessHandlerInstance as a unique_ptr.
    HearthstoneProcessHandler();
    ~HearthstoneProcessHandler();

    void forceStopRecording() override;
private:
    void onProcessStarts(const process_watcher::process::Process& p) override;
    void onProcessStops() override;

    std::unique_ptr<HearthstoneProcessHandlerInstance> _instance;
};

}