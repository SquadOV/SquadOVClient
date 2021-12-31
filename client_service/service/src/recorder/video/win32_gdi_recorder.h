#pragma once

#include "recorder/video/video_recorder.h"

#ifdef _WIN32

#include <Windows.h>
#include <thread>
#include <mutex>

namespace service::recorder::video {

class Win32GdiRecorderInstance : public VideoRecorder {
public:
    explicit Win32GdiRecorderInstance(HWND window);

    void startRecording() override;
    void stopRecording() override;
    
private:
    HWND _window;
    bool _recording = false;
    std::thread _recordingThread;
};

bool tryInitializeWin32GdiRecorder(VideoRecorderPtr& output, const VideoWindowInfo& info, DWORD pid);

}
#endif