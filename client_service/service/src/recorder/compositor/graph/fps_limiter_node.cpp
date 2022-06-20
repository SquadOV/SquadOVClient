#include "recorder/compositor/graph/fps_limiter_node.h"
#include "shared/system/utils.h"
#include "shared/log/log.h"

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
        // Sleeps 16ms - a reasonable amount of time.
        shared::system::utils::preciseSleep(16 * 1000000);
        return;
    }

    // Determine how many frames to write and pass it on to the next step.
    {
        const auto now = service::recorder::encoder::AVSyncClock::now();
        const auto refFrameTime = _startFrameTime.value() + std::chrono::nanoseconds(_nsPerFrame * _frameCounter);
        const auto elapsedFrames = std::ceil(static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(now - refFrameTime).count()) / _nsPerFrame.count());

        if (image) {
            flowToNext(imageContext, image, elapsedFrames, rotation);
        }

        _frameCounter += elapsedFrames;
    }

    // Now we sleep the current thread until we have to move on as determined by the FPS limit.
    {
        const auto now = service::recorder::encoder::AVSyncClock::now();
        const auto nextFrameTime = _startFrameTime.value() + std::chrono::nanoseconds(_nsPerFrame * _frameCounter);
        shared::system::utils::preciseSleep(std::chrono::duration_cast<std::chrono::nanoseconds>(nextFrameTime - now).count());
    }
}

}