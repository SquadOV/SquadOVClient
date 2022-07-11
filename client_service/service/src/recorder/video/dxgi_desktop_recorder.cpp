#include "recorder/video/dxgi_desktop_recorder.h"

#include "recorder/encoder/av_encoder.h"
#include "recorder/image/image.h"
#include "recorder/image/d3d_image.h"
#include "renderer/d3d11_texture.h"
#include "shared/errors/error.h"
#include "shared/log/log.h"
#include "shared/system/win32/hwnd_utils.h"
#include "shared/system/win32/interfaces/win32_system_process_interface.h"
#include "system/settings.h"

#include <iostream>

#ifdef _WIN32
#include <VersionHelpers.h>

#define DISABLE_ALT_TAB_PROTECTION 0
using namespace std::chrono_literals;

namespace service::recorder::video {

DxgiDesktopRecorder::DxgiDesktopRecorder(HWND window, DWORD pid):
    _window(window),
    _pid(pid) {

    // Need to initialize immediately to detect if DXGI isn't supported so we can error out appropriately.
    initialize(false);

    TCHAR windowTitle[1024];
    GetWindowTextA(_window, windowTitle, 1024);
    LOG_INFO("DXGI Desktop Duplication Recording Window: " << windowTitle << "[" << _width << "x" << _height << "]" << std::endl);

    createDefaultTexture();
}

DxgiDesktopRecorder::~DxgiDesktopRecorder() {
}

void DxgiDesktopRecorder::createDefaultTexture() {
    LOG_INFO("Creating DXGI default texture [tmp]..." << std::endl);
    D3D11_TEXTURE2D_DESC desc = { 0 };
    desc.Width = _width;
    desc.Height = _height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;

    wil::com_ptr<ID3D11Texture2D> tmpTexture;
    HRESULT hr = _self->device()->CreateTexture2D(&desc, nullptr, &tmpTexture);
    if (hr != S_OK || !tmpTexture) {
        LOG_ERROR("Failed to create default texture [tmp].");
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mappedData;

    const auto immediate = _self->immediateContext();
    hr = immediate.context()->Map(tmpTexture.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
    if (hr != S_OK) {
        LOG_ERROR("Failed to map default texture: " << hr);
        return;
    }

    uint8_t* dst = reinterpret_cast<uint8_t*>(mappedData.pData);
    for (size_t r = 0; r < _height; ++r) {
        for (size_t i = 0 ; i < mappedData.RowPitch; ++i) {
            if (i % 4 == 3) {
                dst[i] = 255;
            } else {
                dst[i] = 0;
            }
        }
        dst += mappedData.RowPitch;
    }
    immediate.context()->Unmap(tmpTexture.get(), 0);

    LOG_INFO("...Successfully created default texture [tmp]." << std::endl);

    D3D11_TEXTURE2D_DESC sharedDesc = { 0 };
    sharedDesc.Width = _width;
    sharedDesc.Height = _height;
    sharedDesc.MipLevels = 1;
    sharedDesc.ArraySize = 1;
    sharedDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sharedDesc.SampleDesc.Count = 1;
    sharedDesc.Usage = D3D11_USAGE_DEFAULT;
    sharedDesc.BindFlags = 0;
    sharedDesc.CPUAccessFlags = 0;
    sharedDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

    hr = _self->device()->CreateTexture2D(&sharedDesc, nullptr, &_defaultTexture);
    if (hr != S_OK || !_defaultTexture) {
        LOG_ERROR("Failed to create default texture [default].");
        return;
    }

    immediate.context()->CopyResource(_defaultTexture.get(), tmpTexture.get());
    LOG_INFO("...Successfully created default texture [default]." << std::endl);
}

void DxgiDesktopRecorder::initialize(bool checkRunning) {
    LOG_INFO("Initialize DXGI..." << std::endl);
    shared::system::win32::interfaces::Win32SystemProcessInterface win32;
#if !DISABLE_ALT_TAB_PROTECTION
    // Wait for the window to become unminimized so that we can grab the correct monitor.
    while ((IsIconic(_window) || !shared::system::win32::isProcessForeground(_pid)) && (!checkRunning || _recording)) {
        LOG_INFO("...Window is iconic or process is not foreground." << std::endl);

        const auto hwnd = GetForegroundWindow();

        DWORD testPid = 0;
        GetWindowThreadProcessId(hwnd, &testPid);
        LOG_INFO("...Foreground window is: " << win32.getProcessName(testPid) << " - PID: " << testPid << std::endl);

        std::this_thread::sleep_for(std::chrono::milliseconds(500ms));
    }

    if (checkRunning && !_recording) {
        return;
    }

#endif

    LOG_INFO("Continuing DXGI initialization since window/process is foreground..." << std::endl);
    // IDXGIOutput represents a monitor output. We need to grab the output that
    // corresponds to the monitor the input window is on.
    _refMonitor = MonitorFromWindow(_window, MONITOR_DEFAULTTONULL);
    if (!_refMonitor) {
        THROW_ERROR("Failed to get reference monitor.");
    }

    _self = std::make_unique<service::renderer::D3d11SharedContext>(service::renderer::CONTEXT_FLAG_USE_D3D11_1 | service::renderer::CONTEXT_FLAG_VERIFY_DUPLICATE_OUTPUT, _refMonitor);

    wil::com_ptr<IDXGIDevice> dxgiDevice;
    HRESULT hr = _self->device()->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (hr != S_OK) {
        THROW_ERROR("Failed to get IDXGIDevice.");
    }

    wil::com_ptr<IDXGIAdapter> dxgiAdapter;
    hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
    if (hr != S_OK) {
        THROW_ERROR("Failed to get IDXGIAdapter.");
    }

    wil::com_ptr<IDXGIOutput> dxgiOutput;
    UINT outputIndex = 0;
    while (dxgiAdapter->EnumOutputs(outputIndex++, &dxgiOutput) != DXGI_ERROR_NOT_FOUND) {
        hr = dxgiOutput->GetDesc(&_outputDesc);
        if (hr != S_OK) {
            continue;
        }

        if (_outputDesc.Monitor == _refMonitor) {
            break;
        }

        dxgiOutput.reset();
    }

    if (hr != S_OK || !dxgiOutput) {
        THROW_ERROR("Failed to get IDXGIOutput.");
    }
    _rotation = _outputDesc.Rotation;

    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&_dxgiOutput1);
    if (hr != S_OK) {
        THROW_ERROR("Failed to get IDXGIOutput1.");
    }

    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput5), (void**)&_dxgiOutput5);
    if (hr != S_OK) {
        LOG_WARNING("Failed to get IDXGIOutput5...assuming non-Windows 10 [no HDR support]: " << hr << std::endl);
        _dxgiOutput5.reset();
    }

    reacquireDuplicationInterface();

    _width = static_cast<size_t>(_outputDesc.DesktopCoordinates.right - _outputDesc.DesktopCoordinates.left);
    _height = static_cast<size_t>(_outputDesc.DesktopCoordinates.bottom - _outputDesc.DesktopCoordinates.top);
}

void DxgiDesktopRecorder::reacquireDuplicationInterface() {
    _dupl.reset();

    std::vector<DXGI_FORMAT> formats = {
        // This is the default on non-HDR screens
        DXGI_FORMAT_B8G8R8A8_UNORM,
        // This seems to be what gets used on HDR screens (at least on Windows 10?)
        DXGI_FORMAT_R16G16B16A16_FLOAT
    };

    HRESULT hr = _dxgiOutput5 ?
        _dxgiOutput5->DuplicateOutput1(_self->device(), 0, formats.size(), formats.data(), &_dupl) :
        _dxgiOutput1->DuplicateOutput(_self->device(), &_dupl);
    if (hr != S_OK) {
        THROW_ERROR("Failed to duplicate output: " << hr);
    }

    DXGI_OUTDUPL_DESC duplDesc;
    _dupl->GetDesc(&duplDesc);
}

void DxgiDesktopRecorder::startRecording() {
    _recording = true;
    _recordingThread = std::thread([this](){ 
        int64_t count = 0;
        int64_t numReused = 0;
        
        while (_recording) {
            wil::com_ptr<IDXGIResource> desktopResource = nullptr;
            DXGI_OUTDUPL_FRAME_INFO frameInfo;
            bool reuseOldFrame = false;

            // Check if the window somehow drifted to another monitor in which case we should reinitialize.
            HMONITOR testMonitor = MonitorFromWindow(_window, MONITOR_DEFAULTTONULL);
            if (_recording && testMonitor && testMonitor != _refMonitor) {
                LOG_WARNING("Detected monitor drift..." << std::endl);
                try {
                    initialize(true);

                    if (_recording) {
                        createDefaultTexture();
                    }
                } catch (std::exception& ex) {
                    LOG_ERROR("Failed to reinitialize DXGI due to monitor drift: " << ex.what() << std::endl);
                    _dupl.reset();
                    reuseOldFrame = true;
                }
            }

            if (!_recording) {
                break;
            }

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

            bool isAltTabbed = false;
#if !DISABLE_ALT_TAB_PROTECTION
            // We really only care about recording when the user is playing the game so
            // when the window is minimized just ignore what's been recorded.
            if (IsIconic(_window) || !shared::system::win32::isProcessForeground(_pid)) {
                // Re-use old frames here to try and prevent ourselves from grabbing the desktop.
                // Don't just skip the frame as they may cause us to de-sync?
                reuseOldFrame = true;
                isAltTabbed = true;
            }
#endif

            // Need a check to see if reuseOldFrame is not already true because otherwise
            // we could use frameInfo without it being initialized and run into a win32 error.
            if (!reuseOldFrame) {
                reuseOldFrame |= (frameInfo.AccumulatedFrames == 0);
            }
            
            if (!reuseOldFrame) {
                // See: https://github.com/microsoft/Windows-classic-samples/blob/1d363ff4bd17d8e20415b92e2ee989d615cc0d91/Samples/DXGIDesktopDuplication/cpp/DuplicationManager.cpp
                ID3D11Texture2D* tex = nullptr;
                hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&tex);
                if (hr != S_OK) {
                    LOG_INFO("DXGI FAILED TO QUERY INTERFACE: " << hr << std::endl);
                    continue;
                }

                flowToNext(_self.get(), tex, 1, _rotation);
                tex->Release();
            } else if (isAltTabbed && _defaultTexture) {
                flowToNext(_self.get(), _defaultTexture.get(), 1, _rotation);
            } else {
                ++numReused;
            }

            ++count;
        }

        LOG_INFO("DXGI Stats :: Count: " << count << " Reused: " << numReused << std::endl);
    });

    // Make sure the recording thread is at a higher priority
    SetThreadPriority(_recordingThread.native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);
}

void DxgiDesktopRecorder::stopRecording() {
    _recording = false;

    if (_recordingThread.joinable()) {
        _recordingThread.join();
    }
}

bool tryInitializeDxgiDesktopRecorder(VideoRecorderPtr& output, const VideoWindowInfo& info, DWORD pid) {
    if (!IsWindows8OrGreater()) {
        LOG_INFO("Reject DXGI due to < Windows 8." << std::endl);
        return false;
    }

    if (info.isWindowed) {
        LOG_INFO("Rejecting DXGI due to windowed mode." << std::endl);
        return false;
    }

    HWND wnd = shared::system::win32::findWindowForProcessWithMaxDelay(pid, std::chrono::milliseconds(0));
    if (!wnd) {
        LOG_INFO("Rejecting DXGI due to inability to find window." << std::endl);
        return false;
    }

    try {
        output.reset(new DxgiDesktopRecorder(wnd, pid));
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to enable Dxgi Desktop Recording: " << ex.what() << std::endl);
        return false;
    }

    return true;
}

}

#endif