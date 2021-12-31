#pragma once

#include "recorder/compositor/layers/compositor_layer.h"

namespace service::recorder::compositor::layers {

class ImageLayer: public CompositorLayer {
public:
    virtual ID3D11Texture2D* get() const = 0;
};

}