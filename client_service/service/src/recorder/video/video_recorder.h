#pragma once

#include <memory>

namespace service::recorder::encoder {
class AvEncoder;
}

namespace service::recorder::video {

class VideoRecorder {
public:
    virtual ~VideoRecorder() {}
    virtual void startRecording(service::recorder::encoder::AvEncoder* encoder) = 0;
    virtual void stopRecording() = 0;
protected:
    
};
using VideoRecorderPtr = std::unique_ptr<VideoRecorder>;


}