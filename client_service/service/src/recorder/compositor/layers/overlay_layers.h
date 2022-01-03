#pragma once

#include "recorder/compositor/layers/compositor_layer.h"
#include "shared/games.h"
#include "shared/color.h"

#include <nlohmann/json.hpp>
#include <vector>

namespace service::recorder::compositor::layers {

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

std::vector<CompositorLayerPtr> createCompositorLayersForOverlayLayer(const OverlayLayer& layer);

}