#pragma once

#include "recorder/compositor/graph/compositor_node.h"

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

class VideoRecorder: public service::recorder::compositor::graph::CompositorNode {
public:
    virtual ~VideoRecorder() {}
    virtual void startRecording() = 0;
    virtual void stopRecording() = 0;
protected:
    
};
using VideoRecorderPtr = std::unique_ptr<VideoRecorder>;


}