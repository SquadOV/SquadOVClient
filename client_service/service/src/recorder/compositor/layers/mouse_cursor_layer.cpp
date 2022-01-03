#include "recorder/compositor/layers/mouse_cursor_layer.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"
#include <wil/com.h>

namespace service::recorder::compositor::layers {

MouseCursorLayer::MouseCursorLayer(HWND parentWindow):
    _parentWindow(parentWindow)
{
    
}

void MouseCursorLayer::customRender(ID3D11Texture2D* output, IDXGISurface1* surface, HDC hdc) {
    CURSORINFO info = { 0 };
    info.cbSize = sizeof(CURSORINFO);

    if (!GetCursorInfo(&info)) {
        LOG_WARNING("Failed to get cursor info: " << shared::errors::getWin32ErrorAsString() << std::endl);
        return;
    }
    
    const bool isVisible = _enabled && (info.flags & CURSOR_SHOWING);
    if (!isVisible) {
        return;
    }

    // The cursor comes in screen position so we convert it into client position
    // (i.e. relative to the window in question).
    POINT cursorPosition = { info.ptScreenPos.x, info.ptScreenPos. y};
    if (!ScreenToClient(_parentWindow, &cursorPosition)) {
        return;
    }

    // Check whether or not the cursor is out of bounds so we can easily skip.
    RECT windowRes;
    if (!GetClientRect(_parentWindow, &windowRes)) {
        return;
    }

    if (cursorPosition.x < windowRes.left || cursorPosition.x > windowRes.right ||
        cursorPosition.y < windowRes.top || cursorPosition.y > windowRes.bottom) {
        return;
    }

    // It doesn't really matter if this function call fails. We only use it to check the hotspot
    // and if that's 0 whatever.
    ICONINFO iconInfo = { 0 };
    GetIconInfo(info.hCursor, &iconInfo);

    if (!DrawIconEx(
        hdc,                                                        // hdc
        cursorPosition.x - windowRes.left - iconInfo.xHotspot,      // xLeft
        cursorPosition.y - windowRes.top - iconInfo.yHotspot,       // yTop
        info.hCursor,                                               // hIcon (cursor equivalent to icon)
        0,                                                          // cxWidth
        0,                                                          // cyWidth
        0,                                                          // istepIfAniCur
        nullptr,                                                    // hbrFlickerFreeDraw
        DI_NORMAL | DI_DEFAULTSIZE                                  // diFlags
    )) {
        LOG_WARNING("Failed to draw cursor: " << shared::errors::getWin32ErrorAsString() << std::endl);
        return;
    }
}

}