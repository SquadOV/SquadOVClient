#include "recorder/video/win32_gdi_recorder.h"

#include "recorder/image/image.h"
#include "recorder/encoder/av_encoder.h"
#include "system/win32/hwnd_utils.h"
#include "shared/log/log.h"

#include <iostream>
#include <chrono>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

#define LOG_FRAME_TIME 0
#ifdef _WIN32

namespace {

constexpr size_t desiredFps = 60;
constexpr double nsPerFrame = 1.0 / desiredFps * 1e+9;

}

namespace service::recorder::video {

using TickClock = std::chrono::high_resolution_clock;

Win32GdiRecorderInstance::Win32GdiRecorderInstance(HWND window):
    _window(window) {
    
    // For debugging print ouf the window name that we're recording from.
    TCHAR windowTitle[1024];
    GetWindowTextA(_window, windowTitle, 1024);
    LOG_INFO("Win32 GDI Recording Window: " << windowTitle << std::endl);
}

void Win32GdiRecorderInstance::stopRecording() {
    _recording = false;

    if (_recordingThread.joinable()) {
        _recordingThread.join();
    }
}

void Win32GdiRecorderInstance::startRecording(service::recorder::encoder::AvEncoder* encoder) {
    _recording = true;
    _recordingThread = std::thread([this, encoder](){ 
        HDC hdcWindow = GetDC(_window);
        HDC hdcMem = CreateCompatibleDC(hdcWindow);
        HBITMAP hbm = nullptr;
        service::recorder::image::Image frame;

        BITMAPFILEHEADER   bmfHeader;
        BITMAPINFOHEADER   bi;
        bi.biSize = sizeof(BITMAPINFOHEADER);    
        bi.biPlanes = 1;    
        bi.biBitCount = 32;    
        bi.biCompression = BI_RGB;

        bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
        bmfHeader.bfType = 0x4D42; //BM  

        const auto recordStart = TickClock::now();

        while(_recording) {
            const auto startFrame = TickClock::now();
  
            RECT rcClient;
            GetClientRect(_window, &rcClient);

            const auto width = rcClient.right-rcClient.left;
            const auto height = rcClient.bottom-rcClient.top;

            // Means we're probably minimized.
            if (width == 0 || height == 0) {
                continue;
            }

            if (!hbm) {

                hbm = CreateCompatibleBitmap(hdcWindow, width, height);
                frame.initializeImage(width, height);
                bi.biWidth = width;
                // Make bitmaps draw normally (i.e. Y=0 is the top of image...)
                bi.biHeight = -height;
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

            if (!ok) {
                LOG_WARNING("BitBlt failed." << std::endl);
                continue;
            } 
 
            // According to the MSDN docs for GetDIBits we need to clear out the HBITMAP we want to
            // get bits from from any DCs.
            SelectObject(hdcMem, oldHbm);
            GetDIBits(hdcMem, hbm, 0,
                height,
                (void*)frame.buffer(),
                (BITMAPINFO *)&bi,
                DIB_RGB_COLORS);
            
            const auto endCapture = TickClock::now();
#if LOG_FRAME_TIME
            const auto gdiElapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(endCapture - startFrame).count();
#endif

            encoder->addVideoFrame(frame);

            const auto endFrame = TickClock::now();
            const auto numNs = std::chrono::duration_cast<std::chrono::nanoseconds>(endFrame - startFrame).count();
            
#if LOG_FRAME_TIME
            LOG_INFO("Frame Time - GDI:" << gdiElapsed * 1.0e-6 << " + Queue Encode:" << (numMs - gdiElapsed) * 1.0e-6  << std::endl);
#endif

            // top out at 60hz? probably not ever going to be a problem with GDI.
            if (numNs < size_t(nsPerFrame)) {
                std::this_thread::sleep_for(std::chrono::nanoseconds(size_t(nsPerFrame) - numNs));
            }
        }

        DeleteObject(hbm);    
        
        DeleteDC(hdcMem);
        ReleaseDC(_window, hdcWindow);
    });
}

bool tryInitializeWin32GdiRecorder(VideoRecorderPtr& output, DWORD pid) {
    HWND wnd = service::system::win32::findWindowForProcessWithMaxDelay(pid, std::chrono::milliseconds(120000));
    if (!wnd) {
        return false;
    }

    output.reset(new Win32GdiRecorderInstance(wnd));
    return true;
}

}
#endif
