#include "hearthstone/hearthstone_process_handler.h"
#include "shared/log/log.h"

namespace service::hearthstone {

class HearthstoneProcessHandlerInstance {
public:
    HearthstoneProcessHandlerInstance(const process_watcher::process::Process& p);
    ~HearthstoneProcessHandlerInstance();

private:
    process_watcher::process::Process  _process;
};

HearthstoneProcessHandlerInstance::HearthstoneProcessHandlerInstance(const process_watcher::process::Process& p):
    _process(p) {
}

HearthstoneProcessHandlerInstance::~HearthstoneProcessHandlerInstance() {

}

HearthstoneProcessHandler::HearthstoneProcessHandler() = default;
HearthstoneProcessHandler::~HearthstoneProcessHandler() = default;

void HearthstoneProcessHandler::onProcessStarts(const process_watcher::process::Process& p) {
    if (_instance) {
        return;
    }

    LOG_INFO("START HEARTHSTONE" << std::endl);
    _instance = std::make_unique<HearthstoneProcessHandlerInstance>(p);
}

void HearthstoneProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    LOG_INFO("STOP HEARTHSTONE" << std::endl);
    _instance.reset(nullptr);
}

}