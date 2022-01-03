#include "recorder/compositor/graph/fps_limiter_node.h"

namespace service::recorder::compositor::graph {

FpsLimiterNode::FpsLimiterNode(int32_t fps):
    _fps(fps)
{
    _frameCounter = 0;
    _nsPerFrame =  std::chrono::nanoseconds(static_cast<size_t>(1.0 / _fps * 1.0e+9));
}

void FpsLimiterNode::setStartFrameTime(const service::recorder::encoder::AVSyncClock::time_point& tp) {
    std::lock_guard guard(_startMutex);
    _startFrameTime = tp;
    _frameCounter = 0;
}

void FpsLimiterNode::receiveTexture(service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, size_t numFrames, DXGI_MODE_ROTATION rotation) {
    std::shared_lock guard(_startMutex);
    // This node forces us to pass on images down the pipeline to make sure the approximate FPS
    // is as desired. The naive way to do this is to only allow a frame to pass by every (1 / fps) seconds.
    // However, in reality this will give us a lower FPS video than desired since there's no guarantee that the frames
    // will come in from the input at exactly that interval.
    if (!_startFrameTime) {
        return;
    }
    const auto now = service::recorder::encoder::AVSyncClock::now();
    const auto refFrameTime = _startFrameTime.value() + std::chrono::nanoseconds(_nsPerFrame * _frameCounter);
    
    if (now >= refFrameTime) {
        const auto elapsedFrames = std::ceil(static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(now - refFrameTime).count()) / _nsPerFrame.count());
        flowToNext(imageContext, image, elapsedFrames, rotation);
        _frameCounter += elapsedFrames;
    }
}

}