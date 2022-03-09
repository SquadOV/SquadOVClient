#pragma once

#include "recorder/video/video_recorder.h"
#include "recorder/image/d3d_image.h"
#include "renderer/d3d11_context.h"

#ifdef _WIN32

#include <Windows.h>
#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dxgi1_5.h>
#include <thread>
#include <mutex>
#include <vector>
#include <wil/com.h>

namespace service::recorder::video {

class DxgiDesktopRecorder : public VideoRecorder {
public:
    DxgiDesktopRecorder(HWND window, DWORD pid);
    ~DxgiDesktopRecorder();

    void startRecording() override;
    void stopRecording() override;
    
private:
    HWND _window;
    bool _recording = false;    
    std::thread _recordingThread;
    service::renderer::D3d11SharedContextPtr _self;

    void initialize();
    void reacquireDuplicationInterface();
    void createDefaultTexture();

    size_t _width = 0;
    size_t _height = 0;
    DWORD _pid = 0;

    // DX11 and DXGI related pointers.
    wil::com_ptr<IDXGIOutput1> _dxgiOutput1;
    wil::com_ptr<IDXGIOutput5> _dxgiOutput5;
    wil::com_ptr<IDXGIOutputDuplication> _dupl;
    wil::com_ptr<ID3D11Texture2D> _defaultTexture;
    DXGI_OUTPUT_DESC _outputDesc;

    DXGI_MODE_ROTATION _rotation;
};

bool tryInitializeDxgiDesktopRecorder(VideoRecorderPtr& output, const VideoWindowInfo& info, DWORD pid);

}


#endif