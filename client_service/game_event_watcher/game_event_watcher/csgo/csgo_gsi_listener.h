#pragma once

#include <memory>

namespace game_event_watcher {

class CsgoGsiListener {
public:
    static CsgoGsiListener* singleton();
    static void enableCsgoGsi();

    CsgoGsiListener();
    ~CsgoGsiListener();

    int port() const;
};
using CsgoGsiListenerPtr = std::unique_ptr<CsgoGsiListener>;

}