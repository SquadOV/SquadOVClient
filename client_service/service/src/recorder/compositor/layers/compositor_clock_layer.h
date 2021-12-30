#pragma once

#include "recorder/compositor/layers/compositor_layer.h"

namespace service::recorder::compositor::layers {

class CompositorClockLayer: public CompositorLayer {
public:

private:
};

using CompositorClockLayerPtr = std::shared_ptr<CompositorClockLayer>;

}