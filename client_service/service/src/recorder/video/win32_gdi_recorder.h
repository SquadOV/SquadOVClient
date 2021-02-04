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

    void startRecording(size_t fps) override;
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder) override;
    void stopRecording() override;
    
private:
    HWND _window;
    bool _recording = false;
    std::thread _recordingThread;
    service::recorder::encoder::AvEncoder* _activeEncoder = nullptr;
    std::mutex _encoderMutex;
};

bool tryInitializeWin32GdiRecorder(VideoRecorderPtr& output, const VideoWindowInfo& info, DWORD pid);

}
#endif