#pragma once

#include "recorder/process_record_interface.h"
#include "aimlab/aimlab_db_interface.h"
#include "process_watcher/handler.h"
#include "process_watcher/process/process.h"
#include <memory>

namespace service::database {
class DatabaseApi;
}

namespace service::aimlab {

class AimlabProcessHandlerInstance;
class AimlabProcessHandler : public process_watcher::ProcessWatchHandler, public recorder::ProcessRecordInterface {
public:
    AimlabProcessHandler();
    ~AimlabProcessHandler();

    void forceStopRecording() override;

private:
    void onProcessStarts(const process_watcher::process::Process& p) override;
    void onProcessStops() override;

    std::unique_ptr<AimlabProcessHandlerInstance> _instance;
};

}