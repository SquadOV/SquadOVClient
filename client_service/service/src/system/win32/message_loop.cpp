#include "system/win32/message_loop.h"
#include "system/settings.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"
#include "shared/ipc/constants.h"
#include <thread>

namespace service::system::win32 {
namespace {

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return Win32MessageLoop::singleton()->handleMessage(hwnd, uMsg, wParam, lParam);
}

}

Win32MessageLoop* Win32MessageLoop::singleton() {
    static auto singleton = std::make_unique<Win32MessageLoop>();
    return singleton.get();
}

void Win32MessageLoop::start() {
    BOOL ret = 0;
    MSG msg;

    const auto hInst = GetModuleHandle(NULL);

    WNDCLASS wndClass = {};
    wndClass.lpfnWndProc = WindowProc;
    wndClass.lpszClassName = "SQUADOV WND CLASS";
    wndClass.hInstance = hInst;

    const auto wndClassAtom = RegisterClass(&wndClass);
    if (!wndClassAtom) {
        THROW_ERROR("Failed to create SquadOV window class: " << shared::errors::getWin32ErrorAsString());
    }

    HWND msgWnd = CreateWindowEx(
        0,
        wndClass.lpszClassName,
        "SquadOV",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        HWND_MESSAGE,
        nullptr,
        hInst,
        nullptr
    );

    if (!msgWnd) {
        THROW_ERROR("Failed to create SquadOV message window: " << shared::errors::getWin32ErrorAsString());
    }

    while (true) {
        if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            continue;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT Win32MessageLoop::handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case shared::ipc::IPC_KEYPRESS:
        case shared::ipc::IPC_MOUSEPRESS:
            handleKeyboardPress(true);
            return 0;
        case shared::ipc::IPC_KEYRELEASE:
        case shared::ipc::IPC_MOUSERELEASE:
            handleKeyboardPress(false);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Win32MessageLoop::handleKeyboardPress(bool isPress) {
    if (!service::system::getCurrentSettings()->loaded()) {
        return;
    }

    // Translate keypress into actions using the settings.
    const auto keybinds = service::system::getCurrentSettings()->keybinds();

    if (checkKeybindActive(keybinds.pushToTalk) && !_lastPttEnabledState) {
        LOG_INFO("Toggling PTT [ON]" << std::endl);
        _lastPttEnabledState = true;
        notifySquadOvAction(service::system::EAction::PushToTalkEnable);
    } else if (!isPress && _lastPttEnabledState) {
        LOG_INFO("Toggling PTT [OFF]" << std::endl);
        _lastPttEnabledState = false;
        notifySquadOvAction(service::system::EAction::PushToTalkDisable);
    }
}

bool Win32MessageLoop::checkKeybindActive(const std::vector<int>& keybind) {
    if (keybind.empty()) {
        return false;
    }

    bool active = true;
    for (const auto& key: keybind) {
        active &= (GetAsyncKeyState(key) & 0x01);
    }
    return active;
}

int64_t Win32MessageLoop::addActionCallback(service::system::EAction a, const service::system::ActionCallback& cb) {
    std::lock_guard guard(_cbMutex);
    ActionCallbackData data;
    data.id = ++_actionCbCounter;
    data.cb = cb;
    _actionCallbacks[a].emplace_back(std::move(data));
    return _actionCbCounter;
}

void Win32MessageLoop::removeActionCallback(service::system::EAction a, int64_t id) {
    std::lock_guard guard(_cbMutex);
    const auto it = _actionCallbacks.find(a);
    if (it == _actionCallbacks.end()) {
        return;
    }

    const auto idIt = std::find_if(it->second.begin(), it->second.end(), [id](const ActionCallbackData& d){
        return d.id == id;
    });

    if (idIt == it->second.end()) {
        return;
    }

    it->second.erase(idIt);
}

void Win32MessageLoop::notifySquadOvAction(service::system::EAction action) {
    std::lock_guard guard(_cbMutex);
    const auto it = _actionCallbacks.find(action);
    if (it == _actionCallbacks.end()) {
        return;
    }

    for (const auto& cb: it->second) {
        cb.cb();
    }
}

}