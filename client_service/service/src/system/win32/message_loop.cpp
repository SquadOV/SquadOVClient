#include "system/win32/message_loop.h"
#include "system/settings.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"
#include <thread>

namespace shared::system::win32 {
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

    // A HWND that we can use to communicate with the client service via windows messages if necessary.
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

    _virtualKeycodeState.fill(false);

    // Register for raw input to receive WM_INPUT commands so we can handle keybinds and the like.
    RAWINPUTDEVICE rid[2];    
    rid[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
    rid[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
    rid[0].dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;    // adds mouse and also ignores legacy mouse messages
    rid[0].hwndTarget = msgWnd;

    rid[1].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
    rid[1].usUsage = 0x06;              // HID_USAGE_GENERIC_KEYBOARD
    rid[1].dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;    // adds keyboard and also ignores legacy keyboard messages
    rid[1].hwndTarget = msgWnd;

    if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE))) {
        THROW_ERROR("Failed to register raw input devices: " << shared::errors::getWin32ErrorAsString());
    }

    int errCount = 0;
    _lastCheckKeycode = shared::nowUtc();
    while (true) {
        if (GetMessage(&msg, NULL, 0, 0) == -1) {
            LOG_ERROR("Failed to retrieve Win32 Message: " << shared::errors::getWin32ErrorAsString() << std::endl);
            if (++errCount >= 5) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
            continue;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT Win32MessageLoop::handleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_INPUT: {
            UINT dwSize;
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

            std::vector<char> buffer(dwSize);
            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer.data(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
                LOG_WARNING("Failed to read raw input...?" << std::endl);
                return 0;
            }
            
            RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(buffer.data());
            if (raw->header.dwType == RIM_TYPEKEYBOARD) {
                if (raw->data.keyboard.Flags & RI_KEY_BREAK) {
                    _virtualKeycodeState[raw->data.keyboard.VKey] = false;
                } else {
                    _virtualKeycodeState[raw->data.keyboard.VKey] = true;
                }
                onChangeKeycodeState();
            } else if (raw->header.dwType == RIM_TYPEMOUSE) {
                bool newState = false;
                if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN ||
                    raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN ||
                    raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN ||
                    raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN ||
                    raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) {
                    newState = true;
                } else if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP ||
                    raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP ||
                    raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP ||
                    raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP ||
                    raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) {
                    newState = false;
                } else {
                    return 0;
                }

                int vcode = 0;
                if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN ||
                    raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
                    vcode = VK_LBUTTON;
                } else if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN ||
                           raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) {
                    vcode = VK_MBUTTON;
                } else if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN ||
                           raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
                    vcode = VK_RBUTTON;
                } else if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN ||
                           raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP) {
                    vcode = VK_XBUTTON1;
                } else if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN ||
                           raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) {
                    vcode = VK_XBUTTON2;
                } else {
                    return 0;
                }

                _virtualKeycodeState[vcode] = newState;
                onChangeKeycodeState();
            }
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Win32MessageLoop::onChangeKeycodeState() {
    if (!service::system::getCurrentSettings()->loaded()) {
        return;
    }
    const auto now = shared::nowUtc();
    const double deltaT = static_cast<double>(shared::timeToUnixMs(now) - shared::timeToUnixMs(_lastCheckKeycode)) / 1000.0;
    _lastCheckKeycode = now;

    // Translate keypress into actions using the settings.
    const auto keybinds = service::system::getCurrentSettings()->keybinds();
    const auto pttActive = checkKeybindActive(service::system::EKeybindId::Ptt1, keybinds.pushToTalk, deltaT) || checkKeybindActive(service::system::EKeybindId::Ptt2, keybinds.pushToTalk2, deltaT);
    if (pttActive && !_lastPttEnabledState) {
        LOG_INFO("Toggling PTT [ON]" << std::endl);
        _lastPttEnabledState = true;
        notifySquadOvAction(service::system::EAction::PushToTalkEnable);
    } else if (!pttActive && _lastPttEnabledState) {
        LOG_INFO("Toggling PTT [OFF]" << std::endl);
        _lastPttEnabledState = false;
        notifySquadOvAction(service::system::EAction::PushToTalkDisable);
    }

    const auto bookmarkActive = checkKeybindActive(service::system::EKeybindId::Bookmark, keybinds.bookmark, deltaT);
    if (bookmarkActive && _canBookmark) {
        LOG_INFO("User Requested [Bookmark] via Keybinds." << std::endl);
        notifySquadOvAction(service::system::EAction::Bookmark);
        _canBookmark = false;
    } else if (!bookmarkActive) {
        _canBookmark = true;
    }

    const auto clipActive = checkKeybindActive(service::system::EKeybindId::Clip, keybinds.clip, deltaT);
    if (clipActive && _canClip) {
        LOG_INFO("User Requested [Clip] via Keybinds." << std::endl);
        notifySquadOvAction(service::system::EAction::Clip);
        _canClip = false;
    } else if (!clipActive) {
        _canClip = true;
    }
}

bool Win32MessageLoop::checkKeybindActive(service::system::EKeybindId id, const service::system::Keybind& keybind, double deltaT) {
    if (keybind.keys.empty()) {
        return false;
    }

    bool active = true;
    for (const auto& key: keybind.keys) {
        active &= _virtualKeycodeState[key];
    }

    if (!active) {
        _kbHoldSeconds[id] = 0.0;
        return false;
    }

    if (!keybind.mustHold) {
        return true;
    }

    _kbHoldSeconds[id] += deltaT;
    return _kbHoldSeconds[id] >= static_cast<double>(keybind.holdSeconds);
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