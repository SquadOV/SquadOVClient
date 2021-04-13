#include "libinputhooks/inputhooks.h"
#include "shared/ipc/constants.h"
#include <iostream>

HWND targetWindow = nullptr;

LRESULT squadOvKeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
    if (!targetWindow) {
        targetWindow = FindWindowEx(HWND_MESSAGE, NULL, "SQUADOV WND CLASS", "SquadOV");
    }

    const KBDLLHOOKSTRUCT* hook = reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
    PostMessage(targetWindow, (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) ? shared::ipc::IPC_KEYPRESS : shared::ipc::IPC_KEYRELEASE, hook->vkCode, hook->flags);
    return CallNextHookEx(nullptr, code, wParam, lParam);
}