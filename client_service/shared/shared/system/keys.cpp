#include "shared/system/keys.h"


#ifdef _WIN32
#include <Windows.h>
#endif

#include <codecvt>
#include <sstream>

namespace shared::system::keys {

std::optional<std::string> keycodeToName(int keycode) {
    if (keycode >= VK_F13 && keycode <= VK_F24) {
        // Windows apparently doesn't suppor these via GetKeyNameTextW
        std::ostringstream name;
        name << "F" << (13 + keycode - VK_F13);
        return name.str();
    } else if (keycode == VK_RBUTTON) {
        return "MOUSE1";
    } else if (keycode == VK_MBUTTON) {
        return "MOUSE2";
    } else if (keycode == VK_XBUTTON1) {
        return "MOUSE3";
    } else if (keycode == VK_XBUTTON2) {
        return "MOUSE4";
    } else if (keycode == VK_BROWSER_BACK) {
        return "Browser Back";
    } else if (keycode == VK_BROWSER_FORWARD) {
        return "Browser Forward";
    } else if (keycode == VK_BROWSER_REFRESH) {
        return "Browser Refresh";
    } else if (keycode == VK_BROWSER_STOP) {
        return "Browser Stop";
    } else if (keycode == VK_BROWSER_SEARCH) {
        return "Browser Search";
    } else if (keycode == VK_BROWSER_FAVORITES) {
        return "Browser Favorites";
    } else if (keycode == VK_BROWSER_HOME) {
        return "Browser Home";
    } else if (keycode == VK_VOLUME_MUTE) {
        return "Volume Mute";
    } else if (keycode == VK_VOLUME_DOWN) {
        return "Volume Down";
    } else if (keycode == VK_VOLUME_UP) {
        return "Volume Up";
    } else if (keycode == VK_MEDIA_NEXT_TRACK) {
        return "Media Next";
    } else if (keycode == VK_MEDIA_PREV_TRACK) {
        return "Media Previous";
    } else if (keycode == VK_MEDIA_STOP) {
        return "Media Stop";
    } else if (keycode == VK_MEDIA_PLAY_PAUSE) {
        return "Media Play";
    } else {
        UINT scanCode = MapVirtualKeyW(keycode, MAPVK_VK_TO_VSC);
        switch (keycode) {
            case VK_LEFT:
            case VK_UP:
            case VK_RIGHT:
            case VK_DOWN:
            case VK_PRIOR:
            case VK_NEXT:
            case VK_END:
            case VK_HOME:
            case VK_INSERT:
            case VK_DELETE:
            case VK_DIVIDE:
            case VK_NUMLOCK:
                scanCode |= 0x100;
        }

        wchar_t nameBuffer[256];
        const auto result = GetKeyNameTextW(scanCode << 16, nameBuffer, 256);

        std::wstring nameWStr(nameBuffer);
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        if (result > 0) {
            return conv.to_bytes(nameWStr);
        } else {
            std::ostringstream name;
            name << "UNK" << keycode;
            return name.str();
        }
    }

    return std::nullopt;
}

}
