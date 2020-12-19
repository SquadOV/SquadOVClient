#include "wow/wow_process_handler.h"
#include "shared/log/log.h"
#include "game_event_watcher/wow/wow_log_watcher.h"
#include "shared/time.h"

namespace service::wow {

class WoWProcessHandlerInstance {
public:
    WoWProcessHandlerInstance(const process_watcher::process::Process& p);
    ~WoWProcessHandlerInstance();

private:
    process_watcher::process::Process  _process;
    game_event_watcher::WoWLogWatcherPtr _logWatcher;
};

WoWProcessHandlerInstance::WoWProcessHandlerInstance(const process_watcher::process::Process& p):
    _process(p),
    _logWatcher(new game_event_watcher::WoWLogWatcher(true, shared::nowUtc())) {

    
    _logWatcher->loadFromExecutable(p.path());
}

WoWProcessHandlerInstance::~WoWProcessHandlerInstance() {
    
}

WoWProcessHandler::WoWProcessHandler() = default;
WoWProcessHandler::~WoWProcessHandler() = default;

void WoWProcessHandler::onProcessStarts(const process_watcher::process::Process& p) {
    if (_instance) {
        return;
    }

    LOG_INFO("START WOW" << std::endl);
    _instance = std::make_unique<WoWProcessHandlerInstance>(p);
}

void WoWProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    LOG_INFO("STOP WOW" << std::endl);
    _instance.reset(nullptr);
}

}