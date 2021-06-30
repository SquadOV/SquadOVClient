#pragma once

#ifdef _WIN32

#include "renderer/d3d11_renderer.h"
#include "renderer/d3d11_context.h"
#include "renderer/d3d11_model.h"
#include "shared/color.h"

#include <directxmath.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>

namespace service::renderer {
    
using D3d11OverlayComponentPtr = std::shared_ptr<class D3d11OverlayComponent>;
class D3d11OverlayComponent {
public:
    static D3d11OverlayComponentPtr loadFromJson(const nlohmann::json& data, size_t width, size_t height);

    D3d11OverlayComponent(size_t width, size_t height);

    void addToRenderer(D3d11Renderer& renderer, D3d11SharedContext& context);
    void clearCache();

private:
    size_t _canvasWidth = 0;
    size_t _canvasHeight = 0;

    // Rectangle
    struct RectangleData {
        float left;
        float top;
        float width;
        float height;
        float scaleX;
        float scaleY;
        shared::color::ColorRgba color;
    };
    std::optional<RectangleData> _rectangle;
    D3d11ModelPtr _cachedModel;
};

}

#endif