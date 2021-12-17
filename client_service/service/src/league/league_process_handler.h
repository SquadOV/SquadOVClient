#pragma once

#include "recorder/process_record_interface.h"
#include "process_watcher/handler.h"
#include "process_watcher/process/process.h"
#include <memory>

namespace service::league {

class LeagueProcessHandlerInstance;
class LeagueProcessHandler : public process_watcher::ProcessWatchHandler, public recorder::ProcessRecordInterface {
public:
    LeagueProcessHandler();
    ~LeagueProcessHandler();

    void forceStopRecording() override;
private:
    void onProcessStarts(const process_watcher::process::Process& p) override;
    void onProcessStops() override;

    std::unique_ptr<LeagueProcessHandlerInstance> _instance;
};

}