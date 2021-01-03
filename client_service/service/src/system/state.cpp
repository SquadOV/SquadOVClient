#include "system/state.h"

namespace service::system {

GlobalState* getGlobalState() {
    static GlobalStatePtr global = std::make_unique<GlobalState>();
    return global.get();
}

}