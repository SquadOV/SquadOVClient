#pragma once

#include "system/actions.h"
#include "system/keybinds.h"
#include "shared/time.h"
#include <Windows.h>
#include <array>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace shared::system::win32 {

class Win32MessageLoop {
public:
    static Win32MessageLoop* singleton();

    void start();
    int64_t addActionCallback(service::system::EAction a, const service::system::ActionCallback& cb);
    void removeActionCallback(service::system::EAction a, int64_t id);

    LRESULT handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    void onChangeKeycodeState();
    bool checkKeybindActive(service::system::EKeybindId id, const service::system::Keybind& keybind, double deltaT);

    void notifySquadOvAction(service::system::EAction action);

    struct ActionCallbackData {
        int64_t id;
        service::system::ActionCallback cb;
    };
    std::mutex _cbMutex;
    std::unordered_map<service::system::EAction, std::vector<ActionCallbackData>> _actionCallbacks;
    std::unordered_map<service::system::EKeybindId, double> _kbHoldSeconds;
    shared::TimePoint _lastCheckKeycode;
    int64_t _actionCbCounter = 0;
    bool _lastPttEnabledState = false;
    bool _canBookmark = true;
    bool _canClip = true;
    std::array<bool, 256> _virtualKeycodeState;
};

using Win32MessageLoopPtr = std::unique_ptr<Win32MessageLoop>;

}