#pragma once

#include <atomic>
#include <memory>

#include "recorder/image/d3d_image.h"
#include "recorder/encoder/av_encoder.h"

namespace service::recorder::compositor::layers {

class CompositorLayer {
public:
    virtual ~CompositorLayer() {}

    void enable(bool v) { _enabled = v; }
    void updateAt(const service::recorder::encoder::AVSyncClock::time_point& tp) = 0;

protected:
    std::atomic<bool> _enabled = true;
};

using CompositorLayerPtr = std::shared_ptr<CompositorLayer>;

}