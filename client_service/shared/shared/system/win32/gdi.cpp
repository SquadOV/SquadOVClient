#include "shared/system/win32/gdi.h"

namespace shared::system::win32::gdi {

GdiInitializer::GdiInitializer() {
    Gdiplus::GdiplusStartup(&_token, &_input, NULL);
}

GdiInitializer::~GdiInitializer() {
    Gdiplus::GdiplusShutdown(_token);
}

}