#include "libwgc/wgc.h"

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

#include "recorder/image/image.h"

#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#pragma comment(lib, "windowsapp")

#define THROW_ERROR(x) \
    {std::ostringstream _err; \
    _err << x; \
    throw std::runtime_error(_err.str());};

class WindowsGraphicsCaptureImpl {
public:
    WindowsGraphicsCaptureImpl(HWND window);

    void startRecording(size_t fps);
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder);
    void stopRecording();
    
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

using TickClock = std::chrono::high_resolution_clock;

WindowsGraphicsCaptureImpl::WindowsGraphicsCaptureImpl(HWND window):
    _window(window) {
    
    // For debugging print ouf the window name that we're recording from.
    char windowTitle[1024];
    GetWindowTextA(_window, windowTitle, 1024);

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
    _frameArrived = _framePool.FrameArrived(winrt::auto_revoke, { this, &WindowsGraphicsCaptureImpl::onFrameArrived });
}

void WindowsGraphicsCaptureImpl::stopRecording() {
    _frameArrived.revoke();
    _framePool.Close();
    _session.Close();
    _framePool = nullptr;
    _session = nullptr;
    _item = nullptr;   
}

void WindowsGraphicsCaptureImpl::recreateDeviceTextureFromLastSize() {
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

void WindowsGraphicsCaptureImpl::onFrameArrived(
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
        std::cout << "WGC FAILED TO MAP TEXTURE: " << hr << std::endl;
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

void WindowsGraphicsCaptureImpl::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder) {
    std::lock_guard<std::mutex> guard(_encoderMutex);
    _activeEncoder = encoder;
}

void WindowsGraphicsCaptureImpl::startRecording(size_t fps) {
    _session.StartCapture();
}

WindowsGraphicsCaptureItf::WindowsGraphicsCaptureItf(HWND window) {
    _impl = std::make_unique<WindowsGraphicsCaptureImpl>(window);
}

WindowsGraphicsCaptureItf::~WindowsGraphicsCaptureItf() {

}

void WindowsGraphicsCaptureItf::startRecording(size_t fps) {
    _impl->startRecording(fps);
}

void WindowsGraphicsCaptureItf::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder) {
    _impl->setActiveEncoder(encoder);
}

void WindowsGraphicsCaptureItf::stopRecording() {
    _impl->stopRecording();
}

service::recorder::video::VideoRecorder* createWindowsGraphicsCaptureInterface(const service::recorder::video::VideoWindowInfo& info, HWND window) {
    if (!winrt::Windows::Graphics::Capture::GraphicsCaptureSession::IsSupported()) {
        return nullptr;
    }

    return new WindowsGraphicsCaptureItf(window);
}