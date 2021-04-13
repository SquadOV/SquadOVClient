#pragma once

#include "system/actions.h"
#include <Windows.h>
#include <array>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace service::system::win32 {

class Win32MessageLoop {
public:
    static Win32MessageLoop* singleton();

    void start();
    int64_t addActionCallback(service::system::EAction a, const service::system::ActionCallback& cb);
    void removeActionCallback(service::system::EAction a, int64_t id);

    LRESULT handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    void handleKeyboardPress(bool isPress);
    bool checkKeybindActive(const std::vector<int>& keybind);

    void notifySquadOvAction(service::system::EAction action);

    struct ActionCallbackData {
        int64_t id;
        service::system::ActionCallback cb;
    };
    std::mutex _cbMutex;
    std::unordered_map<service::system::EAction, std::vector<ActionCallbackData>> _actionCallbacks;
    int64_t _actionCbCounter = 0;
};

using Win32MessageLoopPtr = std::unique_ptr<Win32MessageLoop>;

}