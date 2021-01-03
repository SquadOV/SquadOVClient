#pragma once

#include <memory>
namespace service::system {

class GlobalState {
public:
    void setPause(bool p) {_paused = p; };
    bool isPaused() const { return _paused; };

private:
    bool _paused = false;
};

using GlobalStatePtr = std::unique_ptr<GlobalState>;

GlobalState* getGlobalState();

}