#pragma once

#include <windows.h>
#include <gdiplus.h>

namespace shared::system::win32::gdi {

class GdiInitializer {
public:
    GdiInitializer();
    ~GdiInitializer();
private:
    ULONG_PTR _token;
    Gdiplus::GdiplusStartupInput _input;
};

}