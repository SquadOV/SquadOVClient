#pragma once

#include "recorder/video/video_recorder.h"

#ifdef _WIN32

#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <thread>

namespace service::recorder::video {

class DxgiDesktopRecorder : public VideoRecorder {
public:
    explicit DxgiDesktopRecorder(HWND window);
    ~DxgiDesktopRecorder();

    void startRecording(service::recorder::encoder::AvEncoder* encoder) override;
    void stopRecording() override;
    
private:
    HWND _window;
    bool _recording = false;    
    std::thread _recordingThread;

    void initialize();
    void reacquireDuplicationInterface();

    size_t _width = 0;
    size_t _height = 0;

    // DX11 and DXGI related pointers.
    ID3D11Device* _device = nullptr;
    ID3D11DeviceContext* _context = nullptr;
    IDXGIOutputDuplication* _dupl = nullptr;
    ID3D11Texture2D* _deviceTexture = nullptr;
    IDXGIOutput1* _dxgiOutput1 = nullptr;
};

bool tryInitializeDxgiDesktopRecorder(VideoRecorderPtr& output, DWORD pid);

}


#endif