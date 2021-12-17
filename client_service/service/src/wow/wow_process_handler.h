#pragma once

#include "recorder/process_record_interface.h"
#include "process_watcher/handler.h"
#include "process_watcher/process/process.h"
#include "shared/time.h"
#include <memory>

namespace service::wow {

class WoWProcessHandlerInstance;
class WoWProcessHandler : public process_watcher::ProcessWatchHandler, public recorder::ProcessRecordInterface {
public:
    WoWProcessHandler();
    ~WoWProcessHandler();

    void forceStopRecording() override;
    void manualStartLogWatching(const std::filesystem::path& path, const std::filesystem::path& vodPath, const shared::TimePoint& vodStartTime);

    bool onProcessChange(const process_watcher::process::Process& p) override;
private:
    void onProcessStarts(const process_watcher::process::Process& p) override;
    void onProcessStops() override;

    std::unique_ptr<WoWProcessHandlerInstance> _instance;
};

}