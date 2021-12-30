#include "recorder/compositor/compositor.h"

namespace service::recorder::compositor {

void Compositor::setClockLayer(const layers::CompositorLayerPtr& layer) {
    _clockLayer = layer;
}

void Compositor::addLayer(const layers::CompositorLayerPtr& layer) {
    _layers.push_back(layer);
}

}