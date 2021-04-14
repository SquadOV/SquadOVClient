#pragma once

#include "recorder/video/video_recorder.h"
#include "renderer/d3d11_context.h"

#ifdef _WIN32

#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <thread>
#include <mutex>

namespace service::recorder::video {

class DxgiDesktopRecorder : public VideoRecorder {
public:
    DxgiDesktopRecorder(HWND window, DWORD pid, service::renderer::D3d11SharedContext* shared);
    ~DxgiDesktopRecorder();

    void startRecording() override;
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder) override;
    void stopRecording() override;
    
private:
    HWND _window;
    bool _recording = false;    
    std::thread _recordingThread;
    service::renderer::D3d11SharedContext* _shared = nullptr;
    service::renderer::D3d11SharedContextPtr _self;

    void initialize();
    void reacquireDuplicationInterface();

    size_t _width = 0;
    size_t _height = 0;
    service::recorder::encoder::AvEncoder* _activeEncoder = nullptr;
    std::mutex _encoderMutex;
    DWORD _pid = 0;

    // DX11 and DXGI related pointers.
    IDXGIOutputDuplication* _dupl = nullptr;
    IDXGIOutput1* _dxgiOutput1 = nullptr;

    DXGI_MODE_ROTATION _rotation;
};

bool tryInitializeDxgiDesktopRecorder(VideoRecorderPtr& output, const VideoWindowInfo& info, DWORD pid, service::renderer::D3d11SharedContext* shared);

}


#endif