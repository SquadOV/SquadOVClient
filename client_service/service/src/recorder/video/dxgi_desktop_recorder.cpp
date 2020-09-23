#include "recorder/video/dxgi_desktop_recorder.h"

#include "recorder/encoder/av_encoder.h"
#include "recorder/image/image.h"
#include "shared/errors/error.h"
#include "system/win32/hwnd_utils.h"

#include <iostream>

#define LOG_FRAME_TIME 0
#ifdef _WIN32

namespace service::recorder::video {

namespace {

constexpr size_t desiredFps = 60;
constexpr double nsPerFrame = 1.0 / desiredFps * 1e+9;

}

using TickClock = std::chrono::high_resolution_clock;

DxgiDesktopRecorder::DxgiDesktopRecorder(HWND window):
    _window(window) {

    // Initialize DirectX 11 and get pointers to various DXGI interfaces so that
    // we can get access to the desktop duplication API.
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &_device,
        nullptr,
        &_context
    );

    if (hr != S_OK) {
        THROW_ERROR("Failed to create D3D11 device.");
    }

    IDXGIDevice* dxgiDevice = nullptr;
    hr = _device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (hr != S_OK) {
        THROW_ERROR("Failed to get IDXGIDevice.");
    }

    IDXGIAdapter* dxgiAdapter = nullptr;
    hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
    if (hr != S_OK) {
        THROW_ERROR("Failed to get IDXGIAdapter.");
    }
    dxgiDevice->Release();
    dxgiDevice = nullptr;

    // IDXGIOutput represents a monitor output. We need to grab the output that
    // corresponds to the monitor the input window is on.
    HMONITOR refMonitor = MonitorFromWindow(_window, MONITOR_DEFAULTTONULL);
    if (!refMonitor) {
        THROW_ERROR("Failed to get reference monitor.");
    }

    // Enforce that the HWND we're referring to is in full screen mode. If the HWND
    // isn't full screen then falling back on GDI is more than sufficient.
    if (!service::system::win32::isFullscreen(_window, refMonitor)) {
        THROW_ERROR("The input HWND is not fullscreen.");
    }

    IDXGIOutput* dxgiOutput = nullptr;
    DXGI_OUTPUT_DESC outputDesc;
    UINT outputIndex = 0;
    while (dxgiAdapter->EnumOutputs(outputIndex++, &dxgiOutput) != DXGI_ERROR_NOT_FOUND) {
        hr = dxgiOutput->GetDesc(&outputDesc);
        if (hr != S_OK) {
            continue;
        }

        if (outputDesc.Monitor == refMonitor) {
            break;
        }

        dxgiOutput->Release();
        dxgiOutput = nullptr;
    }

    dxgiAdapter->Release();
    dxgiAdapter = nullptr;
    if (hr != S_OK) {
        THROW_ERROR("Failed to get IDXGIOutput.");
    }

    IDXGIOutput1* dxgiOutput1 = nullptr;
    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&dxgiOutput1);
    if (hr != S_OK) {
        THROW_ERROR("Failed to get IDXGIOutput1.");
    }

    hr = dxgiOutput1->DuplicateOutput(_device, &_dupl);
    if (hr != S_OK) {
        THROW_ERROR("Failed to duplicate output.");
    }

    dxgiOutput1->Release();
    dxgiOutput1 = nullptr;

    // Create a shared texture that we'll copy the desktop into.
    D3D11_TEXTURE2D_DESC sharedDesc = { 0 };
    sharedDesc.Width = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
    sharedDesc.Height = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;
    sharedDesc.MipLevels = 1;
    sharedDesc.ArraySize = 1;
    sharedDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sharedDesc.SampleDesc.Count = 1;
    sharedDesc.Usage = D3D11_USAGE_STAGING;
    sharedDesc.BindFlags = 0;
    sharedDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    sharedDesc.MiscFlags = 0;

    _width = static_cast<size_t>(sharedDesc.Width);
    _height = static_cast<size_t>(sharedDesc.Height);

    hr = _device->CreateTexture2D(&sharedDesc, nullptr, &_deviceTexture);
    if (hr != S_OK) {
        THROW_ERROR("Failed to create device texture: " << hr);
    }

    TCHAR windowTitle[1024];
    GetWindowTextA(_window, windowTitle, 1024);
    std::cout << "DXGI Desktop Duplication Recording Window: " << windowTitle << "[" << _width << "x" << _height << "]" << std::endl;
}

DxgiDesktopRecorder::~DxgiDesktopRecorder() {
    if (!!_dupl) {
        _dupl->Release();
        _dupl = nullptr;
    }

    if (!!_deviceTexture) {
        _deviceTexture->Release();
        _deviceTexture = nullptr;
    }

    if (!!_context) {
        _context->Release();
        _context = nullptr;
    }

    if (!!_device) {
        _device->Release();
        _device = nullptr;
    }
}

void DxgiDesktopRecorder::startRecording(service::recorder::encoder::AvEncoder* encoder) {
    _recording = true;
    _recordingThread = std::thread([this, encoder](){ 
        service::recorder::image::Image frame;
        frame.initializeImage(_width, _height);

        while (_recording) {
            IDXGIResource* desktopResource = nullptr;
            DXGI_OUTDUPL_FRAME_INFO frameInfo;

            const auto startFrameTm = TickClock::now();

            HRESULT hr = _dupl->AcquireNextFrame(500, &frameInfo, &desktopResource);
            if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
                continue;
            }

            if (hr != S_OK) {
                std::cerr << " WARNING: AcquireNextFrame failed." << std::endl;
                continue;
            }

            const auto postAcquireTm = TickClock::now();

            // I'm not sure why it returns a ID3D11Texture2D but that's what
            // Microsoft's DXGI desktop duplication example does.
            // See: https://github.com/microsoft/Windows-classic-samples/blob/1d363ff4bd17d8e20415b92e2ee989d615cc0d91/Samples/DXGIDesktopDuplication/cpp/DuplicationManager.cpp
            ID3D11Texture2D* tex = nullptr;
            hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&tex);
            desktopResource->Release();
            desktopResource = nullptr;
            if (hr != S_OK) {
                std::cerr << " WARNING: failed to obtain ID3D11Texture2D." << std::endl;
                continue;
            }

            // TODO: More efficient copy using information about which parts of the image
            // are dirty/moved?
            _context->CopyResource(_deviceTexture, tex);
            tex->Release();
            _dupl->ReleaseFrame();

            // Read from the texture onto the CPU and then copy it into our image buffer
            // to send to the encoder.
            D3D11_MAPPED_SUBRESOURCE mappedData;
            hr = _context->Map(_deviceTexture, 0, D3D11_MAP_READ, 0, &mappedData);
            if (hr != S_OK) {
                std::cerr << " WARNING: failed to map image -- " << hr << std::endl;
                continue;
            }

            const uint8_t* src = reinterpret_cast<const uint8_t*>(mappedData.pData);
            uint8_t* dst = frame.buffer();

            for (size_t r = 0; r < _height; ++r) {
                assert(frame.numBytesPerRow() <= mappedData.RowPitch);
                std::memcpy(dst, src, frame.numBytesPerRow());
                src += mappedData.RowPitch;
                dst += frame.numBytesPerRow();
            }

            _context->Unmap(_deviceTexture, 0);

            const auto sendToEncoderTm = TickClock::now();
            encoder->addVideoFrame(frame);

            const auto postEncoderTm = TickClock::now();

#if LOG_FRAME_TIME
            const auto timeToAcquireNs = std::chrono::duration_cast<std::chrono::nanoseconds>(postAcquireTm - startFrameTm).count();
            const auto copyNs = std::chrono::duration_cast<std::chrono::nanoseconds>(sendToEncoderTm - postAcquireTm).count();
            const auto encodeNs = std::chrono::duration_cast<std::chrono::nanoseconds>(postEncoderTm - sendToEncoderTm).count();

            std::cout << "Frame Time - DXGI:" << std::endl
                << "\tAcquire:" << timeToAcquireNs * 1.0e-6 << std::endl
                << "\tCopy:" << copyNs * 1.0e-6  << std::endl
                << "\tEncode:" << encodeNs * 1.0e-6  << std::endl;
#endif

            // Limit our frames to 60hz so that we don't unnecessarily create frames.
            // TODO: Sync this to the encoder Hz.
            const auto elapsedNs = std::chrono::duration_cast<std::chrono::nanoseconds>(postEncoderTm - startFrameTm).count();
            if (elapsedNs < size_t(nsPerFrame)) {
                std::this_thread::sleep_for(std::chrono::nanoseconds(size_t(nsPerFrame) - elapsedNs));
            }
        }
    });
}

void DxgiDesktopRecorder::stopRecording() {
    _recording = false;

    if (_recordingThread.joinable()) {
        _recordingThread.join();
    }
}

bool tryInitializeDxgiDesktopRecorder(VideoRecorderPtr& output, DWORD pid) {
    HWND wnd = service::system::win32::findWindowForProcessWithMaxDelay(pid, std::chrono::milliseconds(120000));
    if (!wnd) {
        return false;
    }

    try {
        output.reset(new DxgiDesktopRecorder(wnd));
    } catch (std::exception& ex) {
        std::cerr << "Failed to enable Dxgi Desktop Recording: " << ex.what() << std::endl;
        return false;
    }

    return true;
}

}

#endif