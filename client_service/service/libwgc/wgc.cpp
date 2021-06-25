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
#include "recorder/image/d3d_image.h"
#include "renderer/d3d11_texture.h"
#include "shared/log/log.h"

#include <atomic>
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
    WindowsGraphicsCaptureImpl(HWND window, service::renderer::D3d11SharedContext* context, bool useGpuFrame);

    void startRecording();
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder);
    void stopRecording();
    
private:
    HWND _window;
    service::recorder::encoder::AvEncoder* _activeEncoder = nullptr;
    std::mutex _encoderMutex;

    std::atomic_bool _running = false;
    bool _useGpuFrame = false;

    // D3D stuff
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice _rtDevice = nullptr;
    service::renderer::D3d11SharedContext* _shared = nullptr;
    service::renderer::D3d11SharedContext _self;
    service::recorder::image::D3dImage _gpuFrame;
    service::recorder::image::Image _cpuFrame;

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

WindowsGraphicsCaptureImpl::WindowsGraphicsCaptureImpl(HWND window, service::renderer::D3d11SharedContext* shared, bool useGpuFrame):
    _window(window),
    _useGpuFrame(useGpuFrame),
    _shared(shared),
    _gpuFrame(shared) {
    
    // For debugging print ouf the window name that we're recording from.
    char windowTitle[1024];
    GetWindowTextA(_window, windowTitle, 1024);

    // I'm honestly just copying code from 
    // https://github.com/microsoft/Windows.UI.Composition-Win32-Samples/blob/master/cpp/ScreenCaptureforHWND/ScreenCaptureforHWND 
    // so no intelligent comments here.
    winrt::com_ptr<::IInspectable> rtDevice;

    IDXGIDevice* dxgiDevice = nullptr;
    HRESULT hr = _self.device()->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (hr != S_OK) {
        THROW_ERROR("Failed to get IDXGIDevice.");
    }

    winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice, rtDevice.put()));
    dxgiDevice->Release();
    dxgiDevice = nullptr;
    
    _rtDevice = rtDevice.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();

    // This is what creates the Windows graphics capture item that we need to use to specify *what* to capture.
    auto activationFactory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
	auto interopFactory = activationFactory.as<IGraphicsCaptureItemInterop>();
	winrt::check_hresult(interopFactory->CreateForWindow(_window, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), reinterpret_cast<void**>(winrt::put_abi(_item))));

    _lastSize = _item.Size();
    _framePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::CreateFreeThreaded(_rtDevice, winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized, 1, _lastSize);
    _frameArrived = _framePool.FrameArrived(winrt::auto_revoke, { this, &WindowsGraphicsCaptureImpl::onFrameArrived });
    _session = _framePool.CreateCaptureSession(_item);
    _gpuFrame.initializeImage(_lastSize.Width, _lastSize.Height);
    _cpuFrame.initializeImage(_lastSize.Width, _lastSize.Height);
}

void WindowsGraphicsCaptureImpl::stopRecording() {
    _running = false;
    if (_frameArrived) {
        _frameArrived.revoke();
    }

    if (_session) {
        _session.Close();
        _session = nullptr;
    }

    if (_framePool) {
        _framePool.Close();
        _framePool = nullptr;
    }
    
    _item = nullptr;

    // Need this to make sure we don't destroy the object with an active encoder mutex.
    std::lock_guard<std::mutex> guard(_encoderMutex);
    _activeEncoder = nullptr;
}

void WindowsGraphicsCaptureImpl::onFrameArrived(
    const winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool& sender,
    const winrt::Windows::Foundation::IInspectable& args) {

    if (!_running) {
        return;
    }

    auto frame = sender.TryGetNextFrame();
    auto frameContentSize = frame.ContentSize();
    const auto changedSize = (frameContentSize.Width != _lastSize.Width || frameContentSize.Height != _lastSize.Height);

    if (changedSize) {
        _lastSize = frameContentSize;
    }

    auto rtSurface = frame.Surface();
    auto access = rtSurface.as<IDirect3DDxgiInterfaceAccess>();
    winrt::com_ptr<ID3D11Texture2D> frameSurface;
    winrt::check_hresult(access->GetInterface(winrt::guid_of<ID3D11Texture2D>(), frameSurface.put_void()));

    D3D11_TEXTURE2D_DESC desc;
    frameSurface->GetDesc(&desc);

    if (desc.Width != _gpuFrame.width() || desc.Height != _gpuFrame.height()) {
        _gpuFrame.initializeImage(desc.Width, desc.Height);
    }

    if (desc.Width != _cpuFrame.width() || desc.Height != _cpuFrame.height()) {
        _cpuFrame.initializeImage(desc.Width, desc.Height);
    }

    {
        std::lock_guard<std::mutex> guard(_encoderMutex);
        if (_activeEncoder) {
            if (_useGpuFrame) {
                service::renderer::SharedD3d11TextureHandle handle(_shared, frameSurface.get(), false);
                _gpuFrame.copyFromGpu(handle.texture(), DXGI_MODE_ROTATION_IDENTITY);
                _activeEncoder->addVideoFrame(_gpuFrame.rawTexture());
            } else {
                service::renderer::SharedD3d11TextureHandle handle(_shared, frameSurface.get(), false);
                auto immediate = _shared->immediateContext();
                _cpuFrame.loadFromD3d11TextureWithStaging(_shared->device(), immediate.context(), handle.texture());
                _activeEncoder->addVideoFrame(_cpuFrame);
            }
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

void WindowsGraphicsCaptureImpl::startRecording() {
    _running = true;
    _session.StartCapture();
}

WindowsGraphicsCaptureItf::WindowsGraphicsCaptureItf(HWND window, service::renderer::D3d11SharedContext* context, bool useGpuFrame) {
    _impl = std::make_unique<WindowsGraphicsCaptureImpl>(window, context, useGpuFrame);
}

WindowsGraphicsCaptureItf::~WindowsGraphicsCaptureItf() {

}

void WindowsGraphicsCaptureItf::startRecording() {
    _impl->startRecording();
}

void WindowsGraphicsCaptureItf::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder) {
    _impl->setActiveEncoder(encoder);
}

void WindowsGraphicsCaptureItf::stopRecording() {
    _impl->stopRecording();
}

service::recorder::video::VideoRecorder* createWindowsGraphicsCaptureInterface(const service::recorder::video::VideoWindowInfo& info, HWND window, service::renderer::D3d11SharedContext* context, bool useGpuFrame) {
    if (!winrt::Windows::Graphics::Capture::GraphicsCaptureSession::IsSupported()) {
        return nullptr;
    }

    return new WindowsGraphicsCaptureItf(window, context, useGpuFrame);
}

BOOL WINAPI DllMain(
    HINSTANCE hinstDll,
    DWORD fdwReason,
    LPVOID lpReserve
) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            shared::log::Log::initializeGlobalLogger("wgc.log");
            break;
    }
    return TRUE;
}