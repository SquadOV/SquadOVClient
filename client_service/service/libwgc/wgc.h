#pragma once

#include "recorder/video/video_recorder.h"
#include "recorder/encoder/av_encoder.h"
#include <Windows.h>

class WindowsGraphicsCaptureImpl;
class WindowsGraphicsCaptureItf: public service::recorder::video::VideoRecorder {
public:
    WindowsGraphicsCaptureItf(HWND window);
    ~WindowsGraphicsCaptureItf();

    void startRecording(size_t fps) override;
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder) override;
    void stopRecording() override;

private:
    std::unique_ptr<WindowsGraphicsCaptureImpl> _impl;
};

extern "C" {
__declspec(dllexport) service::recorder::video::VideoRecorder* __cdecl createWindowsGraphicsCaptureInterface(const service::recorder::video::VideoWindowInfo& info, HWND window);
}