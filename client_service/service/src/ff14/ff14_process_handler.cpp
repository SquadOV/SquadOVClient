#include "ff14/ff14_process_handler.h"
#include "shared/log/log.h"
#include "shared/games.h"
#include "system/state.h"

namespace service::ff14 {

class Ff14ProcessHandlerInstance {
public:
    explicit Ff14ProcessHandlerInstance(const process_watcher::process::Process& p);
    ~Ff14ProcessHandlerInstance();

private:
    process_watcher::process::Process _process;
};

Ff14ProcessHandlerInstance::Ff14ProcessHandlerInstance(const process_watcher::process::Process& p):
    _process(p)
{

}

Ff14ProcessHandlerInstance::~Ff14ProcessHandlerInstance() {

}

Ff14ProcessHandler::Ff14ProcessHandler() = default;
Ff14ProcessHandler::~Ff14ProcessHandler() = default;

void Ff14ProcessHandler::forceStopRecording() {

}

void Ff14ProcessHandler::onProcessStarts(const process_watcher::process::Process& p) {
    if (_instance) {
        return;
    }

    LOG_INFO("START FF14" << std::endl);
    _instance = std::make_unique<Ff14ProcessHandlerInstance>(p);
    service::system::getGlobalState()->markGameRunning(shared::EGame::Ff14, true);
}

void Ff14ProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    LOG_INFO("STOP FF14" << std::endl);
    service::system::getGlobalState()->markGameRunning(shared::EGame::Ff14, false);
    _instance.reset(nullptr);
    LOG_INFO("\tFF14 fully stopped." << std::endl);
}

}