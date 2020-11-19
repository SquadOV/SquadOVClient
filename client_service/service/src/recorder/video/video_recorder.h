#pragma once

#include <memory>

namespace service::recorder::encoder {
class AvEncoder;
}

namespace service::recorder::video {

struct VideoWindowInfo {
    size_t width = 0;
    size_t height = 0;
    bool isWindowed = false;
    bool init = false;
};

class VideoRecorder {
public:
    virtual ~VideoRecorder() {}
    virtual void startRecording(service::recorder::encoder::AvEncoder* encoder) = 0;
    virtual void stopRecording() = 0;
protected:
    
};
using VideoRecorderPtr = std::unique_ptr<VideoRecorder>;


}