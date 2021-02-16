#include "recorder/video/windows_graphics_capture.h"

#include "recorder/image/image.h"
#include "recorder/encoder/av_encoder.h"
#include "system/win32/hwnd_utils.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"

#include <dxgi1_2.h>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Graphics.DirectX.h>

namespace fs = std::filesystem;

#define LOG_FRAME_TIME 0
#ifdef _WIN32

extern "C"
{
    HRESULT __stdcall CreateDirect3D11DeviceFromDXGIDevice(::IDXGIDevice* dxgiDevice,
        ::IInspectable** graphicsDevice);
}

struct __declspec(uuid("A9B3D012-3DF2-4EE3-B8D1-8695F457D3C1"))
    IDirect3DDxgiInterfaceAccess : ::IUnknown
{
    virtual HRESULT __stdcall GetInterface(GUID const& id, void** object) = 0;
};

namespace service::recorder::video {

using TickClock = std::chrono::high_resolution_clock;

WindowsGraphicsCapture::WindowsGraphicsCapture(HWND window):
    _window(window) {
    
    // For debugging print ouf the window name that we're recording from.
    char windowTitle[1024];
    GetWindowTextA(_window, windowTitle, 1024);
    LOG_INFO("Windows Graphics Capture Recording Window: " << windowTitle << std::endl);

    // Windows grahpics capture usese a D3D context so create the device here.
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        _device.put(),
        nullptr,
        _context.put()
    );

    if (hr != S_OK) {
        THROW_ERROR("Failed to create D3D11 device.");
    }

    // I'm honestly just copying code from 
    // https://github.com/microsoft/Windows.UI.Composition-Win32-Samples/blob/master/cpp/ScreenCaptureforHWND/ScreenCaptureforHWND 
    // so no intelligent comments here.
    winrt::com_ptr<::IInspectable> rtDevice;

    auto dxgiDevice = _device.as<IDXGIDevice>();
    winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), rtDevice.put()));
    _rtDevice = rtDevice.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();

    // This is what creates the Windows graphics capture item that we need to use to specify *what* to capture.
    auto activationFactory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
	auto interopFactory = activationFactory.as<IGraphicsCaptureItemInterop>();
	winrt::check_hresult(interopFactory->CreateForWindow(_window, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), reinterpret_cast<void**>(winrt::put_abi(_item))));

    _lastSize = _item.Size();
    recreateDeviceTextureFromLastSize();
    _framePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::CreateFreeThreaded(_rtDevice, winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized, 1, _lastSize);
    _session = _framePool.CreateCaptureSession(_item);
    _frameArrived = _framePool.FrameArrived(winrt::auto_revoke, { this, &WindowsGraphicsCapture::onFrameArrived });
}

void WindowsGraphicsCapture::stopRecording() {
    _frameArrived.revoke();
    _framePool.Close();
    _session.Close();
    _framePool = nullptr;
    _session = nullptr;
    _item = nullptr;   
}

void WindowsGraphicsCapture::recreateDeviceTextureFromLastSize() {
    if (_deviceTexture) {
        _deviceTexture = nullptr;
    }

    D3D11_TEXTURE2D_DESC sharedDesc = { 0 };
    sharedDesc.Width = _lastSize.Width;
    sharedDesc.Height = _lastSize.Height;
    sharedDesc.MipLevels = 1;
    sharedDesc.ArraySize = 1;
    sharedDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sharedDesc.SampleDesc.Count = 1;
    sharedDesc.Usage = D3D11_USAGE_STAGING;
    sharedDesc.BindFlags = 0;
    sharedDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    sharedDesc.MiscFlags = 0;

    if (_device->CreateTexture2D(&sharedDesc, nullptr, _deviceTexture.put()) != S_OK) {
        THROW_ERROR("Failed to create texture.")
    }

    _cpuImage.initializeImage(_lastSize.Width, _lastSize.Height);
}

void WindowsGraphicsCapture::onFrameArrived(
    const winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool& sender,
    const winrt::Windows::Foundation::IInspectable& args) {

    auto frame = sender.TryGetNextFrame();
    auto frameContentSize = frame.ContentSize();
    const auto changedSize = (frameContentSize.Width != _lastSize.Width || frameContentSize.Height != _lastSize.Height);

    if (changedSize) {
        _lastSize = frameContentSize;

        // Need to resize the texture and our frame so we can copy seamlessly.
        recreateDeviceTextureFromLastSize();
    }

    {
        auto rtSurface = frame.Surface();
        auto access = rtSurface.as<IDirect3DDxgiInterfaceAccess>();
        winrt::com_ptr<ID3D11Texture2D> frameSurface;
        winrt::check_hresult(access->GetInterface(winrt::guid_of<ID3D11Texture2D>(), frameSurface.put_void()));
        _context->CopyResource(_deviceTexture.get(), frameSurface.get());
    }

    // Read from the texture onto the CPU and then copy it into our image buffer
    // to send to the encoder.
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HRESULT hr = _context->Map(_deviceTexture.get(), 0, D3D11_MAP_READ, 0, &mappedData);
    if (hr != S_OK) {
        LOG_INFO("WGC FAILED TO MAP TEXTURE: " << hr << std::endl);
    }

    const uint8_t* src = reinterpret_cast<const uint8_t*>(mappedData.pData);
    uint8_t* dst = _cpuImage.buffer();

    for (int32_t r = 0; r < _lastSize.Height; ++r) {
        assert(_cpuImage.numBytesPerRow() <= mappedData.RowPitch);
        std::memcpy(dst, src, _cpuImage.numBytesPerRow());
        src += mappedData.RowPitch;
        dst += _cpuImage.numBytesPerRow();
    }
    _context->Unmap(_deviceTexture.get(), 0);

    {
        std::lock_guard<std::mutex> guard(_encoderMutex);
        if (_activeEncoder) {
            _activeEncoder->addVideoFrame(_cpuImage);
        }
    }

    if (changedSize) {
        _framePool.Recreate(_rtDevice, winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized, 1, _lastSize);
    }

}

void WindowsGraphicsCapture::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder) {
    std::lock_guard<std::mutex> guard(_encoderMutex);
    _activeEncoder = encoder;
}

void WindowsGraphicsCapture::startRecording(size_t fps) {
    _session.StartCapture();
}

bool tryInitializeWindowsGraphicsCapture(VideoRecorderPtr& output, const VideoWindowInfo& info, DWORD pid) {
    if (!winrt::Windows::Graphics::Capture::GraphicsCaptureSession::IsSupported()) {
        LOG_INFO("Rejecting Windows Graphics Capture: Not Supported." << std::endl);
        return false;
    }

    if (!info.isWindowed) {
        LOG_INFO("Rejecting Windows Graphics Capture due to non-windowed mode." << std::endl);
        return false;
    }

    HWND wnd = service::system::win32::findWindowForProcessWithMaxDelay(pid, std::chrono::milliseconds(120000));
    if (!wnd) {
        LOG_INFO("Rejecting Windows Graphics Capture due to inability to find window." << std::endl);
        return false;
    }

    output.reset(new WindowsGraphicsCapture(wnd));
    return true;
}

}
#endif