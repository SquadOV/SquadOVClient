#include "recorder/compositor/layers/watermark_layer.h"
#include "shared/filesystem/common_paths.h"
#include "shared/log/log.h"
#include "shared/http/http_client.h"

#include <boost/algorithm/string.hpp>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/files.h>
#include <cryptopp/md5.h>
#include <cryptopp/base64.h>

namespace fs = std::filesystem;
namespace service::recorder::compositor::layers {

std::optional<std::string> WatermarkLayer::_cachedWatermarkHash;

WatermarkLayer::WatermarkLayer(const service::system::WatermarkSettings& watermark):
    _watermark(watermark)
{
    _imageLayer = std::make_shared<ShapeLayer>(ShapeData{Shape::Rectangle, shared::color::ColorRgba{0.f, 0.f, 0.f, 0.f}});
}

WatermarkLayer::~WatermarkLayer() {
}

void WatermarkLayer::initializeWatermark(service::renderer::D3d11Renderer* renderer) {
    // We should cache the watermark though (which does introduce some mechanisms of bypass). The only possible thing
    // we can do here is to cache it to a file and store an MD5 in memory and when we go back to load it up double check the hash.
    // Granted, a user could find and modify the MD5 hash in memory using a memory editor or something...but good for them.
    const auto cachedImagePath = shared::filesystem::getSquadOvTempFolder() / fs::path("watermark.png");
    
    size_t maxTryCounter = 0;
    while(!fs::exists(cachedImagePath) || !_cachedWatermarkHash || computeHash(cachedImagePath) != _cachedWatermarkHash.value()) {
        if (maxTryCounter >= 5) {
            // Yeah, if users somehow bypass this shit, just let it keep functioning rather than erroring out here.
            // I'd say it's not the end of the world.
            LOG_ERROR("Failed to initialize watermark.");
            return;
        }

        // Download into cache and store the Md5.
        _cachedWatermarkHash = downloadWatermarkTo(cachedImagePath);
        ++maxTryCounter;
    }

    _imageLayer->initializeImage(renderer, cachedImagePath);
    _imageLayer->setOpacity(0.3f);

    const float height = _watermark.size * 2.f;
    const float heightPixels = height / 2.f * renderer->height();

    // Note that we have to go into screen coordinates first to compute what the width should be in NDC.
    // Doing a straight aspect ratio scale on the height in WDC doesn't really work since -1 to 1 in width
    // means something entirely different in height.
    const float widthPixels = heightPixels * _imageLayer->width() / _imageLayer->height();
    const float width = widthPixels / renderer->width() * 2.f;
    _imageLayer->setScale(DirectX::XMFLOAT3(width / 2.f, height / 2.f, 1.f));

    // Note that the full screen squad has its anchor point in the center.
    const float targetX = (_watermark.xPos  == service::system::SquadOvPositionX::Left) ? -1.0f + width / 2.f :
        (_watermark.xPos  == service::system::SquadOvPositionX::Right) ? 1.0f - width / 2.f:
        0.f;
    const float targetY = (_watermark.yPos  == service::system::SquadOvPositionY::Top) ? 1.0f - width / 2.f :
        (_watermark.yPos  == service::system::SquadOvPositionY::Bottom) ? -1.0f + height / 2.f:
        0.f;
    
    _imageLayer->setTranslation(DirectX::XMFLOAT3(targetX, targetY, 0.f));
}

std::string WatermarkLayer::downloadWatermarkTo(const std::filesystem::path& fname) const {
    // Load the watermark from a URL. This way we generally don't have to worry about the user tampering with
    // a file on their local machine and by-passing the watermark. The file should be small enough that I don't need
    // to worry that the user's internet will be severely impacted by retrieving this file.
    const auto watermarkUrl = "https://us-central1.content.squadov.gg/SquadOV_Gradient_White.png";
    if (!shared::http::downloadFileToLocation(watermarkUrl, fname, [](size_t, size_t, size_t, size_t){})) {
        return "";
    }
    return computeHash(fname);
}

std::string WatermarkLayer::computeHash(const std::filesystem::path& fname) const {
    if (!fs::exists(fname)) {
        return "";
    }
    CryptoPP::Weak::MD5 hash;
    std::string digest;
    
    CryptoPP::FileSource file(
        fname.c_str(),
        true,
        new CryptoPP::HashFilter(hash, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(digest)))
    );

    boost::algorithm::trim(digest);
    return digest;
}

void WatermarkLayer::updateAt(const service::recorder::encoder::AVSyncClock::time_point& tp, service::renderer::D3d11Renderer* renderer, ID3D11DeviceContext* context) {
    if (!_imageLayer) {
        return;
    }
    _imageLayer->updateAt(tp, renderer, context);
}

void WatermarkLayer::finalizeAssetsForRenderer(service::renderer::D3d11Renderer* renderer) {
    initializeWatermark(renderer);

    if (!_imageLayer) {
        return;
    }
    _imageLayer->finalizeAssetsForRenderer(renderer);
}

}