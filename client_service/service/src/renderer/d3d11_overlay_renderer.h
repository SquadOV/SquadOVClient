#pragma once

#ifdef _WIN32

#include <atomic>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <unordered_set>
#include <vector>

#include "shared/games.h"
#include "recorder/image/image.h"
#include "recorder/image/d3d_image.h"
#include "renderer/d3d11_model.h"
#include "renderer/d3d11_context.h"
#include "renderer/d3d11_renderer.h"
#include "renderer/d3d11_overlay_layer.h"

namespace service::renderer {

struct OverlayLayer {
    bool enabled = false;
    std::string name;
    std::unordered_set<shared::EGame> games;
    nlohmann::json fabric;
    size_t width = 0;
    size_t height = 0;

    static OverlayLayer fromJson(const nlohmann::json& obj);
};

struct OverlaySettings {
    bool enabled = false;
    std::vector<OverlayLayer> layers;

    static OverlaySettings fromJson(const nlohmann::json& obj);
};


struct D3d11ResourceDeviceTuple {
    ID3D11Texture2D* texture;
    D3d11SharedContext* context;
};

class D3d11OverlayRenderer {
public:
    D3d11OverlayRenderer(const OverlaySettings& settings, shared::EGame game);
    void initializeCpu();
    void initializeGpu();
    void reinitializeRenderer(size_t width, size_t height);

    void enable(bool enable);

    void render(const service::recorder::image::ImagePtr& output, const service::recorder::image::Image& input);
    void render(const service::recorder::image::ImagePtr& output, ID3D11Texture2D* input, D3d11SharedContext* inputContext);

    void render(const service::recorder::image::D3dImagePtr& output, const service::recorder::image::Image& input);
    void render(const service::recorder::image::D3dImagePtr& output, ID3D11Texture2D* input, D3d11SharedContext* inputContext);

private:
    OverlaySettings _settings;
    std::atomic<bool> _enabled = false;
    D3d11SharedContextPtr _context;

    mutable std::mutex _renderMutex;
    D3d11RendererPtr _renderer;

    D3d11ResourceDeviceTuple loadIntoTmpImage(service::recorder::image::D3dImagePtr& tmp, const service::recorder::image::Image& image, bool copy);
    D3d11ResourceDeviceTuple loadIntoTmpOutput(const service::recorder::image::Image& image);
    service::recorder::image::D3dImagePtr _tmpOutputImage;

    D3d11ResourceDeviceTuple loadIntoTmpInput(ID3D11Texture2D* input, D3d11SharedContext* inputContext);
    D3d11ResourceDeviceTuple loadIntoTmpInput(const service::recorder::image::Image& image);
    service::recorder::image::D3dImagePtr _tmpInputImage;

    void render(const D3d11ResourceDeviceTuple& output, const D3d11ResourceDeviceTuple& input) const;

    D3d11ModelPtr _outputQuad;
    std::vector<D3d11OverlayLayerPtr> _layers;
};

using D3d11OverlayRendererPtr = std::shared_ptr<D3d11OverlayRenderer>;

}

#endif