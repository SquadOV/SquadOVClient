#pragma once

#include <memory>

#include "recorder/image/d3d_image.h"

namespace service::recorder::compositor::layers {

class CompositorLayer {
public:
};

using CompositorLayerPtr = std::shared_ptr<CompositorLayer>;

}