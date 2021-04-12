#include "recorder/video/dxgi_desktop_recorder.h"

#include "recorder/encoder/av_encoder.h"
#include "recorder/image/image.h"
#include "recorder/image/d3d_image.h"
#include "renderer/d3d11_texture.h"
#include "shared/errors/error.h"
#include "shared/log/log.h"
#include "system/win32/hwnd_utils.h"
#include "system/settings.h"

#include <iostream>

#define LOG_FRAME_TIME 0
#ifdef _WIN32
#include <VersionHelpers.h>

using namespace std::chrono_literals;

namespace service::recorder::video {

using TickClock = std::chrono::high_resolution_clock;

DxgiDesktopRecorder::DxgiDesktopRecorder(HWND window, service::renderer::D3d11SharedContext* shared):
    _window(window),
    // Note that there's two D3d11SharedContext objects here.
    // I found that putting the desktop duplication on the shared context that we use for rendering
    // and the like will completely freeze the pipeline. So instead, everything on the desktop duplication
    // API will use the _self context instead.
    _shared(shared),
    _self() {

    // Need to initialize immediately to detect if DXGI isn't supported so we can error out appropriately.
    initialize();

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
}

void DxgiDesktopRecorder::initialize() {
    IDXGIDevice* dxgiDevice = nullptr;
    HRESULT hr = _self.device()->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
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
    while (IsIconic(_window) || !service::system::win32::isWindowTopmost(_window)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100ms));
    }

    // IDXGIOutput represents a monitor output. We need to grab the output that
    // corresponds to the monitor the input window is on.
    HMONITOR refMonitor = MonitorFromWindow(_window, MONITOR_DEFAULTTONULL);
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
    if (hr != S_OK || !dxgiOutput) {
        THROW_ERROR("Failed to get IDXGIOutput.");
    }
    _rotation = outputDesc.Rotation;

    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&_dxgiOutput1);
    if (hr != S_OK) {
        THROW_ERROR("Failed to get IDXGIOutput1.");
    }

    reacquireDuplicationInterface();

    _width = static_cast<size_t>(outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left);
    _height = static_cast<size_t>(outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top);

    dxgiOutput->Release();
}

void DxgiDesktopRecorder::reacquireDuplicationInterface() {
    if (!!_dupl) {
        _dupl->Release();
        _dupl = nullptr;        
    }

    HRESULT hr = _dxgiOutput1->DuplicateOutput(_self.device(), &_dupl);
    if (hr != S_OK) {
        THROW_ERROR("Failed to duplicate output: " << hr);
    }
}

void DxgiDesktopRecorder::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder) {
    std::lock_guard<std::mutex> guard(_encoderMutex);
    _activeEncoder = encoder;
}

void DxgiDesktopRecorder::startRecording() {
    _recording = true;
    _recordingThread = std::thread([this](){ 
        // We create a GPU image and a CPU image and let the user to change which one they'd rather use to send output to.
        service::recorder::image::D3dImage hwFrame(_shared);
        hwFrame.initializeImage(_width, _height, true);

        service::recorder::image::Image cpuFrame;
        cpuFrame.initializeImage(_width, _height);

        service::system::getCurrentSettings()->reloadSettingsFromFile();
        const bool useHwFrame = service::system::getCurrentSettings()->recording().useVideoHw;

        int64_t count = 0;
        int64_t numReused = 0;
        
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
            const auto result = !!_dupl ? _dupl->ReleaseFrame() : DXGI_ERROR_ACCESS_LOST;
            if (result == DXGI_ERROR_ACCESS_LOST) {
                // Don't need to error out here since we only start to use _dupl after the
                // call to ReleaseFrame.
                try {
                    reacquireDuplicationInterface();
                } catch (std::exception& ex) {
                    LOG_WARNING("Failed to reacquire duplication interface [release frame]...retrying: " << ex.what() << std::endl);
                    reuseOldFrame = true;
                }
            }

            HRESULT hr = !!_dupl ? _dupl->AcquireNextFrame(100, &frameInfo, &desktopResource) : DXGI_ERROR_WAIT_TIMEOUT;
            if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
                reuseOldFrame = true;
                ++numReused;
            } else {
                if (hr == DXGI_ERROR_ACCESS_LOST) {
                    LOG_INFO("DXGI Access Lost." << std::endl);
                    try {
                        reacquireDuplicationInterface();
                    } catch (std::exception& ex) {
                        LOG_WARNING("Failed to reacquire duplication interface [DXGI_ERROR_ACCESS_LOST]...retrying: " << ex.what() << std::endl);
                    }
                }

                if (hr != S_OK) {
                    LOG_INFO("DXGI NOT OK:" << hr << std::endl);
                    reuseOldFrame = true;
                }
            }

            // We really only care about recording when the user is playing the game so
            // when the window is minimized just ignore what's been recorded.
            if (IsIconic(_window) || !service::system::win32::isWindowTopmost(_window)) {
                // Re-use old frames here to try and prevent ourselves from grabbing the desktop.
                // Don't just skip the frame as they may cause us to de-sync?
                reuseOldFrame = true;
            }

            reuseOldFrame |= (frameInfo.AccumulatedFrames == 0);

            const auto postAcquireTm = TickClock::now();
            
            if (!reuseOldFrame) {
                // I'm not sure why it returns a ID3D11Texture2D but that's what
                // Microsoft's DXGI desktop duplication example does.
                // See: https://github.com/microsoft/Windows-classic-samples/blob/1d363ff4bd17d8e20415b92e2ee989d615cc0d91/Samples/DXGIDesktopDuplication/cpp/DuplicationManager.cpp
                ID3D11Texture2D* tex = nullptr;
                hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&tex);
                if (hr != S_OK) {
                    LOG_INFO("DXGI FAILED TO QUERY INTERFACE: " << hr << std::endl);
                    continue;
                }

                if (useHwFrame) {
                    hwFrame.copyFromSharedGpu(&_self, tex, _rotation);
                } else {
                    auto immediate = _self.immediateContext();
                    cpuFrame.loadFromD3d11TextureWithStaging(_self.device(), immediate.context(), tex);
                }
                tex->Release();
            }

            if (desktopResource) {
                desktopResource->Release();
                desktopResource = nullptr;
            }

            const auto sendToEncoderTm = TickClock::now();
            {
                std::lock_guard<std::mutex> guard(_encoderMutex);
                if (_activeEncoder) {
                    if (useHwFrame) {
                        _activeEncoder->addVideoFrame(hwFrame.rawTexture());
                    } else {
                        _activeEncoder->addVideoFrame(cpuFrame);
                    }
                }
            }

            const auto postEncoderTm = TickClock::now();
            
#if LOG_FRAME_TIME
            const auto timeToAcquireNs = std::chrono::duration_cast<std::chrono::nanoseconds>(postAcquireTm - startFrameTm).count();
            const auto copyNs = std::chrono::duration_cast<std::chrono::nanoseconds>(sendToEncoderTm - postAcquireTm).count();
            const auto encodeNs = std::chrono::duration_cast<std::chrono::nanoseconds>(postEncoderTm - sendToEncoderTm).count();

            LOG_INFO("Frame Time - DXGI:" << (timeToAcquireNs + copyNs + encodeNs) * 1.0e-6 << " [" << frameInfo.AccumulatedFrames << " frames]" << std::endl
                << "\tAcquire:" << timeToAcquireNs * 1.0e-6 << std::endl
                << "\tCopy:" << copyNs * 1.0e-6  << std::endl
                << "\tEncode:" << encodeNs * 1.0e-6  << std::endl);
#endif

            const auto elapsedNs = std::chrono::duration_cast<std::chrono::nanoseconds>(postEncoderTm - startFrameTm);
            ++count;
        }

        LOG_INFO("DXGI Stats :: Count: " << count << " Reused: " << numReused << std::endl);
    });
}

void DxgiDesktopRecorder::stopRecording() {
    _recording = false;

    if (_recordingThread.joinable()) {
        _recordingThread.join();
    }
}

bool tryInitializeDxgiDesktopRecorder(VideoRecorderPtr& output, const VideoWindowInfo& info, DWORD pid, service::renderer::D3d11SharedContext* shared) {
    if (!IsWindows8OrGreater()) {
        LOG_INFO("Reject DXGI due to < Windows 8." << std::endl);
        return false;
    }

    if (info.isWindowed) {
        LOG_INFO("Rejecting DXGI due to windowed mode." << std::endl);
        return false;
    }

    HWND wnd = service::system::win32::findWindowForProcessWithMaxDelay(pid, std::chrono::milliseconds(0));
    if (!wnd) {
        LOG_INFO("Rejecting DXGI due to inability to find window." << std::endl);
        return false;
    }

    try {
        output.reset(new DxgiDesktopRecorder(wnd, shared));
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to enable Dxgi Desktop Recording: " << ex.what() << std::endl);
        return false;
    }

    return true;
}

}

#endif