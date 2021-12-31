#pragma once

#include "recorder/compositor/graph/compositor_node.h"
#include "recorder/encoder/av_encoder.h"
#include <chrono>
#include <optional>
#include <shared_mutex>

namespace service::recorder::compositor::graph {

class FpsLimiterNode: public CompositorNode {
public:
    explicit FpsLimiterNode(int32_t fps);
    void setStartFrameTime(const service::recorder::encoder::AVSyncClock::time_point& tp);

protected:
    void receiveTexture(service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, size_t numFrames, DXGI_MODE_ROTATION rotation) override;

private:
    std::shared_mutex _startMutex;
    std::optional<service::recorder::encoder::AVSyncClock::time_point> _startFrameTime;

    int32_t _fps = 0;
    std::chrono::nanoseconds _nsPerFrame;
    
    size_t _frameCounter;
};

using FpsLimiterNodePtr = std::shared_ptr<FpsLimiterNode>;

}