#pragma once

#ifdef _WIN32

#include "renderer/d3d11_renderer.h"
#include "renderer/d3d11_context.h"
#include "renderer/d3d11_overlay_component.h"

#include <memory>
#include <nlohmann/json.hpp>
#include <vector>

namespace service::renderer {

using D3d11OverlayLayerPtr = std::shared_ptr<class D3d11OverlayLayer>;
class D3d11OverlayLayer {
public:
    // Input width height is the size of the canvas that this data was created from
    static D3d11OverlayLayerPtr loadFromJson(const nlohmann::json& data, size_t width, size_t height);

    void initializeWidthHeight(size_t width, size_t height);
    void addToRenderer(D3d11Renderer& renderer, D3d11SharedContext& context);

private:
    // This width and height are the sizes of the output texture.
    size_t _width = 0;
    size_t _height = 0;
    std::vector<D3d11OverlayComponentPtr> _components;
};

}

#endif