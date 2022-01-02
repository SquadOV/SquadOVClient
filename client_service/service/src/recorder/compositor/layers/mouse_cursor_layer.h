#pragma once

#include "recorder/compositor/layers/compositor_layer.h"

namespace service::recorder::compositor::layers {

class MouseCursorLayer: public CompositorLayer {
public:
    explicit MouseCursorLayer(HWND parentWindow);

private:
    HWND _parentWindow;
};

}