#include "renderer/d3d11_overlay_layer.h"

namespace service::renderer {

D3d11OverlayLayerPtr D3d11OverlayLayer::loadFromJson(const nlohmann::json& data, size_t width, size_t height) {
    if (!data.contains("objects") || data.count("objects") == 0) {
        return nullptr;
    }

    auto layer = std::make_shared<D3d11OverlayLayer>();

    const auto& objects = data["objects"];
    for (const auto& obj : objects) {
        auto component = D3d11OverlayComponent::loadFromJson(obj, width, height);
        if (!component) {
            continue;
        }
        layer->_components.push_back(component);
    }

    return layer;
}

void D3d11OverlayLayer::initializeWidthHeight(size_t width, size_t height) {
    if (_width == width && _height == height) {
        return;
    }

    _width = width;
    _height = height;

    // Changing the width height of the output requires us to change how we create the objects so
    // clear out all the cached overlay components as well.
    for (const auto& component : _components) {
        component->clearCache();
    }
}

void D3d11OverlayLayer::addToRenderer(D3d11Renderer& renderer, D3d11SharedContext& context) {
    for (const auto& component : _components) {
        component->addToRenderer(renderer, context);
    }
}

}