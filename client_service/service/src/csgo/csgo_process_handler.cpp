#include "csgo/csgo_process_handler.h"
#include "game_event_watcher/csgo/csgo_log_watcher.h"
#include "recorder/game_recorder.h"
#include "shared/log/log.h"
#include "system/state.h"

namespace service::csgo {

class CsgoProcessHandlerInstance {
public:
    CsgoProcessHandlerInstance(const process_watcher::process::Process& p);
    ~CsgoProcessHandlerInstance();

private:
    game_event_watcher::CsgoLogWatcherPtr _logWatcher;
    process_watcher::process::Process  _process;
    service::recorder::GameRecorderPtr _recorder;
};

CsgoProcessHandlerInstance::CsgoProcessHandlerInstance(const process_watcher::process::Process& p):
    _logWatcher(std::make_unique<game_event_watcher::CsgoLogWatcher>(shared::nowUtc())),
    _process(p) {
    _recorder = std::make_unique<service::recorder::GameRecorder>(_process, shared::EGame::CSGO);
}

CsgoProcessHandlerInstance::~CsgoProcessHandlerInstance() {

}

CsgoProcessHandler::CsgoProcessHandler() = default;
CsgoProcessHandler::~CsgoProcessHandler() = default;

void CsgoProcessHandler::onProcessStarts(const process_watcher::process::Process& p) {
    if (_instance) {
        return;
    }
    LOG_INFO("START CS:GO" << std::endl);
    service::system::getGlobalState()->markGameRunning(shared::EGame::CSGO, true);
    _instance = std::make_unique<CsgoProcessHandlerInstance>(p);
}

void CsgoProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    LOG_INFO("STOP CS:GO" << std::endl);
    service::system::getGlobalState()->markGameRunning(shared::EGame::CSGO, false);
    _instance.reset(nullptr);
}

}