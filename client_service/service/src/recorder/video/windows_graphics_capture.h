#pragma once

#include "recorder/video/video_recorder.h"

#ifdef _WIN32

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <Windows.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <Windows.Graphics.Capture.Interop.h>
#include <windows.graphics.capture.h>

#include "recorder/image/image.h"

#include <thread>
#include <mutex>
#pragma comment(lib, "windowsapp")

namespace service::recorder::video {

class WindowsGraphicsCapture : public VideoRecorder {
public:
    explicit WindowsGraphicsCapture(HWND window);

    void startRecording(size_t fps) override;
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder) override;
    void stopRecording() override;
    
private:
    HWND _window;
    service::recorder::encoder::AvEncoder* _activeEncoder = nullptr;
    std::mutex _encoderMutex;

    // D3D stuff
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice _rtDevice = nullptr;
    winrt::com_ptr<ID3D11Device> _device = nullptr;
    winrt::com_ptr<ID3D11DeviceContext> _context = nullptr;
    winrt::com_ptr<ID3D11Texture2D> _deviceTexture = nullptr;
    service::recorder::image::Image _cpuImage;
    void recreateDeviceTextureFromLastSize();

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

bool tryInitializeWindowsGraphicsCapture(VideoRecorderPtr& output, const VideoWindowInfo& info, DWORD pid);

}
#endif