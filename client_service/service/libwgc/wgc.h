#pragma once

#include "recorder/video/video_recorder.h"
#include "recorder/encoder/av_encoder.h"
#include "renderer/d3d11_context.h"

#include <d3d11_4.h>
#include <dxgi1_2.h>
#include <dxgi1_6.h>
#include <Windows.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <Windows.Graphics.Capture.Interop.h>
#include <windows.graphics.capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Graphics.DirectX.h>

class WindowsGraphicsCaptureItf: public service::recorder::video::VideoRecorder {
public:
    explicit WindowsGraphicsCaptureItf(HWND window);

    void startRecording() override;
    void stopRecording() override;

private:
    HWND _window;
    winrt::Windows::Graphics::DirectX::DirectXPixelFormat _format = winrt::Windows::Graphics::DirectX::DirectXPixelFormat::Unknown;

    std::atomic_bool _running = false;

    // D3D stuff
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice _rtDevice = nullptr;
    service::renderer::D3d11SharedContext _self;

    // Windows Graphics Capture Stuff
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem _item{ nullptr };
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession _session{ nullptr };
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool _framePool{ nullptr };
    winrt::Windows::Graphics::SizeInt32 _lastSize;
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::FrameArrived_revoker _frameArrived;

    void onFrameArrived(
        winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
        winrt::Windows::Foundation::IInspectable const& args);
};

extern "C" {
__declspec(dllexport) service::recorder::video::VideoRecorder* __cdecl createWindowsGraphicsCaptureInterface(const service::recorder::video::VideoWindowInfo& info, HWND window);
}