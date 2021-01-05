#include "recorder/video/dxgi_desktop_recorder.h"

#include "recorder/encoder/av_encoder.h"
#include "recorder/image/image.h"
#include "shared/errors/error.h"
#include "shared/log/log.h"
#include "system/win32/hwnd_utils.h"

#include <iostream>

#define LOG_FRAME_TIME 0
#ifdef _WIN32

using namespace std::chrono_literals;

namespace service::recorder::video {

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

    TCHAR windowTitle[1024];
    GetWindowTextA(_window, windowTitle, 1024);
    LOG_INFO("DXGI Desktop Duplication Recording Window: " << windowTitle << "[" << _width << "x" << _height << "]" << std::endl);
}

DxgiDesktopRecorder::~DxgiDesktopRecorder() {
    if (!!_dupl) {
        _dupl->Release();
        _dupl = nullptr;
    }

    if (!!_dxgiOutput1) {
        _dxgiOutput1->Release();
        _dxgiOutput1 = nullptr;
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

void DxgiDesktopRecorder::initialize() {
    IDXGIDevice* dxgiDevice = nullptr;
    HRESULT hr = _device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
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

    // Wait for the window to become unminimized so that we can grab the correct monitor.
    while (IsIconic(_window)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100ms));
    }

    // IDXGIOutput represents a monitor output. We need to grab the output that
    // corresponds to the monitor the input window is on.
    HMONITOR refMonitor = MonitorFromWindow(_window, MONITOR_DEFAULTTOPRIMARY);
    if (!refMonitor) {
        THROW_ERROR("Failed to get reference monitor.");
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

    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&_dxgiOutput1);
    if (hr != S_OK) {
        THROW_ERROR("Failed to get IDXGIOutput1.");
    }

    reacquireDuplicationInterface();
    
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
}

void DxgiDesktopRecorder::reacquireDuplicationInterface() {
    if (!!_dupl) {
        _dupl->Release();
        _dupl = nullptr;        
    }

    HRESULT hr = _dxgiOutput1->DuplicateOutput(_device, &_dupl);
    if (hr != S_OK) {
        THROW_ERROR("Failed to duplicate output.");
    }
}

void DxgiDesktopRecorder::startRecording(service::recorder::encoder::AvEncoder* encoder, size_t fps) {
    const auto nsPerFrame = std::chrono::nanoseconds(static_cast<size_t>(1.0 / fps * 1.0e+9));

    _recording = true;
    _recordingThread = std::thread([this, encoder, nsPerFrame](){ 
        // Only initialize once we get into the recording thread since we'll need to wait
        // for the window to be not minimizd. This is fine since we'll just be writing black
        // frames until the window is unminimized which is fine!
        initialize();

        service::recorder::image::Image frame;
        frame.initializeImage(_width, _height);

        while (_recording) {
            IDXGIResource* desktopResource = nullptr;
            DXGI_OUTDUPL_FRAME_INFO frameInfo;
            bool reuseOldFrame = false;

            const auto startFrameTm = TickClock::now();

            // MSDN recommends calling ReleaseFrame right before AcquireNextFrame for performance reasons.
            /*
                SOURCE: https://docs.microsoft.com/en-us/windows/win32/api/dxgi1_2/nf-dxgi1_2-idxgioutputduplication-releaseframe
                When the client does not own the frame, the operating system copies all desktop updates to the surface. 
                This can result in wasted GPU cycles if the operating system updates the same region for each frame that occurs.
                When the client acquires the frame, the client is aware of only the final update to this region;
                therefore, any overlapping updates during previous frames are wasted. When the client acquires a frame,
                the client owns the surface; therefore, the operating system can track only the updated regions and
                cannot copy desktop updates to the surface. Because of this behavior, we recommend that you
                minimize the time between the call to release the current frame and the call to acquire the next frame.
            */
            const auto result = _dupl->ReleaseFrame();
            if (result == DXGI_ERROR_ACCESS_LOST) {
                reacquireDuplicationInterface();
                // Don't need to continue here since we only start to use _dupl after the
                // call to ReleaseFrame.
            }

            HRESULT hr = _dupl->AcquireNextFrame(10, &frameInfo, &desktopResource);
            if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
                reuseOldFrame = true;
            } else {
                if (hr == DXGI_ERROR_ACCESS_LOST) {
                    LOG_INFO("DXGI Access Lost." << std::endl);
                    reacquireDuplicationInterface();
                    continue;
                }

                if (hr != S_OK) {
                    LOG_INFO("DXGI NOT OK:" << hr << std::endl);
                    continue;
                }
            }

            // We really only care about recording when the user is playing the game so
            // when the window is minimized just ignore what's been recorded.
            if (IsIconic(_window)) {
                desktopResource->Release();
                LOG_INFO("DXGI IS ICONIC:" << hr << std::endl);
                std::this_thread::sleep_for(nsPerFrame);
                continue;
            }

            reuseOldFrame |= (frameInfo.AccumulatedFrames == 0);

            const auto postAcquireTm = TickClock::now();

            if (!reuseOldFrame) {
                // I'm not sure why it returns a ID3D11Texture2D but that's what
                // Microsoft's DXGI desktop duplication example does.
                // See: https://github.com/microsoft/Windows-classic-samples/blob/1d363ff4bd17d8e20415b92e2ee989d615cc0d91/Samples/DXGIDesktopDuplication/cpp/DuplicationManager.cpp
                ID3D11Texture2D* tex = nullptr;
                hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&tex);
                desktopResource->Release();
                desktopResource = nullptr;
                if (hr != S_OK) {
                    LOG_INFO("DXGI FAILED TO QUERY INTERFACE: " << hr << std::endl);
                    continue;
                }

                // TODO: More efficient copy using information about which parts of the image
                // are dirty/moved?
                _context->CopyResource(_deviceTexture, tex);
                tex->Release();

                // Read from the texture onto the CPU and then copy it into our image buffer
                // to send to the encoder.
                D3D11_MAPPED_SUBRESOURCE mappedData;
                hr = _context->Map(_deviceTexture, 0, D3D11_MAP_READ, 0, &mappedData);
                if (hr != S_OK) {
                    LOG_INFO("DXGI FAILED TO MAP TEXTURE: " << hr << std::endl);
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
            }

            const auto sendToEncoderTm = TickClock::now();
            encoder->addVideoFrame(frame);

            const auto postEncoderTm = TickClock::now();

#if LOG_FRAME_TIME
            const auto timeToAcquireNs = std::chrono::duration_cast<std::chrono::nanoseconds>(postAcquireTm - startFrameTm).count();
            const auto copyNs = std::chrono::duration_cast<std::chrono::nanoseconds>(sendToEncoderTm - postAcquireTm).count();
            const auto encodeNs = std::chrono::duration_cast<std::chrono::nanoseconds>(postMapTm - sendToEncoderTm).count();

            LOG_INFO("Frame Time - DXGI:" << (timeToAcquireNs + copyNs + encodeNs) * 1.0e-6 << " [" << frameInfo.AccumulatedFrames << " frames]" << std::endl
                << "\tAcquire:" << timeToAcquireNs * 1.0e-6 << std::endl
                << "\tCopy:" << copyNs * 1.0e-6  << std::endl
                << "\tEncode:" << encodeNs * 1.0e-6  << std::endl);
#endif

            const auto elapsedNs = std::chrono::duration_cast<std::chrono::nanoseconds>(postEncoderTm - startFrameTm);
            if (elapsedNs < nsPerFrame) {
                std::this_thread::sleep_for(nsPerFrame - elapsedNs);
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

bool tryInitializeDxgiDesktopRecorder(VideoRecorderPtr& output, const VideoWindowInfo& info, DWORD pid) {
    if (info.isWindowed) {
        LOG_INFO("Rejecting DXGI due to windowed mode." << std::endl);
        return false;
    }

    HWND wnd = service::system::win32::findWindowForProcessWithMaxDelay(pid, std::chrono::milliseconds(120000));
    if (!wnd) {
        LOG_INFO("Rejecting DXGI due to inability to find window." << std::endl);
        return false;
    }

    try {
        output.reset(new DxgiDesktopRecorder(wnd));
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to enable Dxgi Desktop Recording: " << ex.what() << std::endl);
        return false;
    }

    return true;
}

}

#endif