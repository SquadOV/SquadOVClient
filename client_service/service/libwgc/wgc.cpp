#include "libwgc/wgc.h"

#include "recorder/image/image.h"
#include "recorder/image/d3d_image.h"
#include "renderer/d3d11_texture.h"
#include "shared/log/log.h"
#include "shared/system/win32/hwnd_utils.h"

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

WindowsGraphicsCaptureItf::WindowsGraphicsCaptureItf(HWND window):
    _window(window)
{
    
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

    // We need to determine what format we want the texture in. If we're doing HDR capture then we want R16G16B16A16Float.
    // If we want SDR capture then we can just use B8G8R8A8UIntNormalized.
    HMONITOR monitor = MonitorFromWindow(_window, MONITOR_DEFAULTTOPRIMARY);
    _format = shared::system::win32::isHDREnabledForMonitor(monitor) ?
        winrt::Windows::Graphics::DirectX::DirectXPixelFormat::R16G16B16A16Float :
        winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized;

    _lastSize = _item.Size();
    _framePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::CreateFreeThreaded(_rtDevice, _format, 1, _lastSize);
    _frameArrived = _framePool.FrameArrived(winrt::auto_revoke, { this, &WindowsGraphicsCaptureItf::onFrameArrived });
    _session = _framePool.CreateCaptureSession(_item);
}

void WindowsGraphicsCaptureItf::stopRecording() {
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
}

void WindowsGraphicsCaptureItf::onFrameArrived(
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

    flowToNext(&_self, frameSurface.get(), 1, DXGI_MODE_ROTATION_IDENTITY);
    if (changedSize) {
        _framePool.Recreate(_rtDevice, _format, 1, _lastSize);
    }
}

void WindowsGraphicsCaptureItf::startRecording() {
    _running = true;
    _session.StartCapture();
}

service::recorder::video::VideoRecorder* createWindowsGraphicsCaptureInterface(const service::recorder::video::VideoWindowInfo& info, HWND window) {
    if (!winrt::Windows::Graphics::Capture::GraphicsCaptureSession::IsSupported()) {
        return nullptr;
    }

    return new WindowsGraphicsCaptureItf(window);
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