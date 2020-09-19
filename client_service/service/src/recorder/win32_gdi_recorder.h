#pragma once

#include "recorder/game_recorder.h"

#ifdef _WIN32

#include <Windows.h>
#include <filesystem>
#include <thread>

namespace service::recorder {

class Win32GdiRecorderInstance : public RecorderInstance {
public:
    explicit Win32GdiRecorderInstance(HWND window);

    void startRecording(video::VideoEncoder* encoder) override;
    void stopRecording() override;
    
private:
    HWND _window;
    bool _recording = false;
    std::thread _recordingThread;
};

bool tryInitializeWin32GdiRecorder(RecorderInstancePtr& output, DWORD pid);

}
#endif