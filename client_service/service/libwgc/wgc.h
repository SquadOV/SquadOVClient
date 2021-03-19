#pragma once

#include "recorder/video/video_recorder.h"
#include "recorder/encoder/av_encoder.h"
#include "renderer/d3d11_context.h"

#include <Windows.h>

class WindowsGraphicsCaptureImpl;
class WindowsGraphicsCaptureItf: public service::recorder::video::VideoRecorder {
public:
    WindowsGraphicsCaptureItf(HWND window, service::renderer::D3d11SharedContext* context, bool useGpuFrame);
    ~WindowsGraphicsCaptureItf();

    void startRecording() override;
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder) override;
    void stopRecording() override;

private:
    std::unique_ptr<WindowsGraphicsCaptureImpl> _impl;
};

extern "C" {
__declspec(dllexport) service::recorder::video::VideoRecorder* __cdecl createWindowsGraphicsCaptureInterface(const service::recorder::video::VideoWindowInfo& info, HWND window, service::renderer::D3d11SharedContext* context, bool useGpuFrame);
}