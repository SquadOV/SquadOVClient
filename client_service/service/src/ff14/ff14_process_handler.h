#pragma once

#include "recorder/process_record_interface.h"
#include "process_watcher/handler.h"
#include "process_watcher/process/process.h"
#include "shared/time.h"
#include <memory>

namespace service::ff14 {

class Ff14ProcessHandlerInstance;
class Ff14ProcessHandler : public process_watcher::ProcessWatchHandler, public recorder::ProcessRecordInterface {
public:
    Ff14ProcessHandler();
    ~Ff14ProcessHandler();

    void forceStopRecording() override;
private:
    void onProcessStarts(const process_watcher::process::Process& p) override;
    void onProcessStops() override;

    std::unique_ptr<Ff14ProcessHandlerInstance> _instance;
};

}