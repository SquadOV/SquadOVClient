#include "recorder/video/win32_gdi_recorder.h"

#include "recorder/image/image.h"
#include "recorder/encoder/av_encoder.h"
#include "renderer/d3d11_context.h"
#include "shared/system/win32/hwnd_utils.h"
#include "shared/log/log.h"

#include <iostream>
#include <chrono>
#include <filesystem>
#include <sstream>

#include <VersionHelpers.h>

namespace fs = std::filesystem;

#ifdef _WIN32

namespace service::recorder::video {

Win32GdiRecorderInstance::Win32GdiRecorderInstance(HWND window):
    _window(window) {
    
    // For debugging print ouf the window name that we're recording from.
    char windowTitle[1024];
    GetWindowTextA(_window, windowTitle, 1024);
    LOG_INFO("Win32 GDI Recording Window: " << windowTitle << std::endl);
}

void Win32GdiRecorderInstance::stopRecording() {
    _recording = false;

    if (_recordingThread.joinable()) {
        _recordingThread.join();
    }
}

void Win32GdiRecorderInstance::startRecording() {
    _recording = true;
    _recordingThread = std::thread([this](){ 
        HDC hdcWindow = GetDC(_window);
        HDC hdcMem = CreateCompatibleDC(hdcWindow);
        HBITMAP hbm = nullptr;
        service::recorder::image::Image frame;
        service::recorder::image::D3dImage hwFrame(service::renderer::getSharedD3d11Context());

        BITMAPFILEHEADER   bmfHeader;
        BITMAPINFOHEADER   bi;
        bi.biSize = sizeof(BITMAPINFOHEADER);    
        bi.biPlanes = 1;    
        bi.biBitCount = 32;    
        bi.biCompression = BI_RGB;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
        bmfHeader.bfType = 0x4D42; //BM  

        while(_recording) {
  
            RECT rcClient;
            GetClientRect(_window, &rcClient);

            const auto width = rcClient.right-rcClient.left;
            const auto height = rcClient.bottom-rcClient.top;
            bool reuseFrame = false;

            // Means we're probably minimized.
            if (IsIconic(_window) || width == 0 || height == 0) {
                reuseFrame = true;
            }

            if (!reuseFrame) {
                if (!hbm || width != frame.width() || height != frame.height()) {
                    if (hbm) {
                        DeleteObject(hbm);
                    }
                    hbm = CreateCompatibleBitmap(hdcWindow, width, height);
                    frame.initializeImage(width, height);
                    hwFrame.initializeImage(width, height, true);
                    bi.biWidth = width;
                    // Make bitmaps draw normally (i.e. Y=0 is the top of image...)
                    bi.biHeight = -height;
                    bi.biSizeImage = width * height * 4;
                }

                const DWORD dwBmpSize = width * height * 4;
                const DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
                bmfHeader.bfSize = dwSizeofDIB;

                HBITMAP oldHbm = (HBITMAP)SelectObject(hdcMem, hbm);
                BOOL ok = BitBlt(
                    hdcMem,
                    0, 0,
                    width, height,
                    hdcWindow,
                    rcClient.left, rcClient.top,
                    SRCCOPY
                );

                if (ok) {
                    // According to the MSDN docs for GetDIBits we need to clear out the HBITMAP we want to
                    // get bits from from any DCs.
                    SelectObject(hdcMem, oldHbm);
                    GetDIBits(hdcMem, hbm, 0,
                        height,
                        (void*)frame.buffer(),
                        (BITMAPINFO *)&bi,
                        DIB_RGB_COLORS);
                } else {
                    LOG_WARNING("BitBlt failed." << std::endl);
                    continue;
                }

                // Is this slow and inefficient? Yes. But this is the GDI pipeline which
                // really shouldn't be used in the case where performance is necessary.
                hwFrame.copyFromCpu(frame);

                flowToNext(service::renderer::getSharedD3d11Context(), hwFrame.rawTexture());
            }
        }

        DeleteObject(hbm);
        
        DeleteDC(hdcMem);
        ReleaseDC(_window, hdcWindow);
    });
}

bool tryInitializeWin32GdiRecorder(VideoRecorderPtr& output, const VideoWindowInfo& info, DWORD pid) {
    const auto recordingSettings = service::system::getCurrentSettings()->recording();
    
    // This also handles the edge case that the user is using a version older than Windows 10
    if (recordingSettings.useWGC2 && IsWindows10OrGreater()) {
        LOG_INFO("Rejecting GDI due to user preference of WGC" << std::endl);
        return false;
    }

    if (!info.isWindowed) {
        LOG_INFO("Rejecting GDI due to non-windowed mode." << std::endl);
        return false;
    }

    HWND wnd = shared::system::win32::findWindowForProcessWithMaxDelay(pid, std::chrono::milliseconds(0));
    if (!wnd) {
        LOG_INFO("Rejecting GDI due to inability to find window." << std::endl);
        return false;
    }

    output.reset(new Win32GdiRecorderInstance(wnd));
    return true;
}

}
#endif
