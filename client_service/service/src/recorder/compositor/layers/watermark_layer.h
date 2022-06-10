#pragma once

#include "recorder/compositor/layers/gdi_image_layer.h"
#include "system/settings.h"
#include <filesystem>
#include <memory>
#include <optional>

namespace service::recorder::compositor::layers {

class WatermarkLayer: public CompositorLayer {
public:
    explicit WatermarkLayer(const service::system::WatermarkSettings& watermark);
    ~WatermarkLayer();

    void updateAt(const service::recorder::encoder::AVSyncClock::time_point& tp, service::renderer::D3d11Renderer* renderer) override;
    void finalizeAssetsForRenderer(service::renderer::D3d11Renderer* renderer) override;
    void customRender(ID3D11Texture2D* output, IDXGISurface1* surface, HDC hdc) override;

private:
    static std::optional<std::string> _cachedWatermarkHash;

    void initializeWatermark(service::renderer::D3d11Renderer* renderer);
    // Downloads and returns the hash.
    std::string downloadWatermarkTo(const std::filesystem::path& fname) const;
    std::string computeHash(const std::filesystem::path& fname) const;

    service::system::WatermarkSettings _watermark;
    std::unique_ptr<GdiImageLayer> _imageLayer;
};

}