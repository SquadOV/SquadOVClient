#include "libinputhooks/inputhooks.h"
#include "shared/ipc/constants.h"
#include <iostream>

HWND targetWindow = nullptr;

void findTargetWindowIfNeeded() {
    if (!targetWindow) {
        targetWindow = FindWindowEx(HWND_MESSAGE, NULL, "SQUADOV WND CLASS", "SquadOV");
    }
}

LRESULT squadOvKeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
    findTargetWindowIfNeeded();

    const KBDLLHOOKSTRUCT* hook = reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
    PostMessage(targetWindow, (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) ? shared::ipc::IPC_KEYPRESS : shared::ipc::IPC_KEYRELEASE, hook->vkCode, hook->flags);
    return CallNextHookEx(nullptr, code, wParam, lParam);
}


LRESULT squadOvMouseProc(int code, WPARAM wParam, LPARAM lParam) {
    findTargetWindowIfNeeded();

    const MSLLHOOKSTRUCT* hook = reinterpret_cast<const MSLLHOOKSTRUCT *>(lParam);

    DWORD vcode = 0;
    bool isPress = false;
    const WORD mouseData = hook->mouseData >> sizeof(WORD);

    switch (wParam) {
        case WM_XBUTTONDOWN:
        case WM_NCXBUTTONDOWN:
            isPress = true;
            vcode = (mouseData == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2;
            break;
        case WM_XBUTTONUP:
        case WM_NCXBUTTONUP:
            isPress = false;
            vcode = (mouseData == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2;
            break;
        case WM_RBUTTONDOWN:
        case WM_NCRBUTTONDOWN:
            isPress = true;
            vcode = VK_RBUTTON;
            break;
        case WM_RBUTTONUP:
        case WM_NCRBUTTONUP:
            isPress = false;
            vcode = VK_RBUTTON;
            break;
        case WM_MBUTTONDOWN:
        case WM_NCMBUTTONDOWN:
            isPress = true;
            vcode = VK_MBUTTON;
            break;
        case WM_MBUTTONUP:
        case WM_NCMBUTTONUP:
            isPress = false;
            vcode = VK_MBUTTON;
            break;
    }

    if (vcode != 0) {
        PostMessage(targetWindow, isPress ? shared::ipc::IPC_MOUSEPRESS : shared::ipc::IPC_MOUSERELEASE, vcode, hook->flags);
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}