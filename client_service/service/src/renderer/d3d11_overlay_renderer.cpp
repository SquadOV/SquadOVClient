#include "renderer/d3d11_overlay_renderer.h"
#include "shared/errors/error.h"

#ifdef _WIN32

namespace service::renderer {
namespace {

D3d11ResourceDeviceTuple d3dImageToTuple(const service::recorder::image::D3dImagePtr& output) {
    auto context = output->context();
    D3d11ResourceDeviceTuple tuple;
    tuple.context = output->context();
    tuple.texture = output->rawTexture();
    return tuple;
}

}

OverlaySettings OverlaySettings::fromJson(const nlohmann::json& obj) {
    OverlaySettings settings;
    settings.enabled = obj.value("enabled", false);

    if (obj.count("layers") > 0) {
        for (const auto& val : obj["layers"]) {
            settings.layers.push_back(OverlayLayer::fromJson(val));
        }
    }
    return settings;
}

OverlayLayer OverlayLayer::fromJson(const nlohmann::json& obj) {
    OverlayLayer layer;
    layer.enabled = obj.value("enabled", false);
    layer.name = obj.value("name", "");
    if (obj.count("games") > 0) {
        for (const auto& val : obj["games"]) {
            layer.games.insert(static_cast<shared::EGame>(val.get<int32_t>()));
        }
    }
    layer.fabric = obj["fabric"];
    layer.width = obj.value("width", 0);
    layer.height = obj.value("height", 0);
    return layer;
}

D3d11OverlayRenderer::D3d11OverlayRenderer(const OverlaySettings& settings, shared::EGame game):
    _settings(settings),
    _enabled(settings.enabled) {

    // Force all the objects within each layers and translate them into real objects to render.
    // We need to render them in the reverse order they show up, this way the 'first' layer shows
    // up on top.
    for (size_t i = 0; i < _settings.layers.size(); ++i) {
        const auto realIdx = _settings.layers.size() - 1 - i;
        const auto& layer = _settings.layers[i];

        if (!layer.enabled) {
            continue;
        }

        if (layer.games.find(game) == layer.games.end()) {
            continue;
        }

        _layers.push_back(D3d11OverlayLayer::loadFromJson(layer.fabric, layer.width, layer.height));
    }
}

void D3d11OverlayRenderer::initializeCpu() {
    _context = std::make_unique<D3d11SharedContext>(CONTEXT_FLAG_USE_D3D11_1, (HMONITOR)NULL, D3d11Device::CPU);
}

void D3d11OverlayRenderer::initializeGpu() {
    _context = std::make_unique<D3d11SharedContext>(CONTEXT_FLAG_USE_D3D11_1, (HMONITOR)NULL, D3d11Device::GPU);
}

void D3d11OverlayRenderer::reinitializeRenderer(size_t width, size_t height) {
    std::lock_guard guard(_renderMutex);
    _renderer = std::make_unique<D3d11Renderer>(_context.get());
    _renderer->initializeRenderer(width, height);

    _outputQuad = service::renderer::createFullScreenQuad(_context->device());
    _renderer->addModelToScene(_outputQuad);

    for (const auto& layer: _layers) {
        if (!layer) {
            continue;
        }

        layer->initializeWidthHeight(width, height);
        layer->addToRenderer(*_renderer, *_context);
    }
}

void D3d11OverlayRenderer::enable(bool enable) {
    _enabled = enable;
}

void D3d11OverlayRenderer::render(const service::recorder::image::ImagePtr& output, const service::recorder::image::Image& input) {
    std::lock_guard guard(_renderMutex);
    if (!_enabled) {
        output->copyFrom(input);
        return;
    }

    const D3d11ResourceDeviceTuple inputTuple = loadIntoTmpInput(input);
    const D3d11ResourceDeviceTuple outputTuple = loadIntoTmpOutput(*output);

    render(outputTuple, inputTuple);

    auto immediate = outputTuple.context->immediateContext();
    output->loadFromD3d11TextureWithStaging(outputTuple.context->device(), immediate.context(), outputTuple.texture);
}

void D3d11OverlayRenderer::render(const service::recorder::image::ImagePtr& output, ID3D11Texture2D* input, D3d11SharedContext* inputContext) {
    std::lock_guard guard(_renderMutex);

    if (!_enabled) {
        auto immediate = inputContext->immediateContext();
        output->loadFromD3d11TextureWithStaging(inputContext->device(), immediate.context(), input);
        return;
    }

    const D3d11ResourceDeviceTuple inputTuple = loadIntoTmpInput(input, inputContext);
    const D3d11ResourceDeviceTuple outputTuple = loadIntoTmpOutput(*output);

    render(outputTuple, inputTuple);

    auto immediate = outputTuple.context->immediateContext();
    output->loadFromD3d11TextureWithStaging(outputTuple.context->device(), immediate.context(), outputTuple.texture);
}

void D3d11OverlayRenderer::render(const service::recorder::image::D3dImagePtr& output, const service::recorder::image::Image& input) {
    std::lock_guard guard(_renderMutex);
    if (!_enabled) {
        output->copyFromCpu(input);
        return;
    }

    const D3d11ResourceDeviceTuple inputTuple = loadIntoTmpInput(input);
    const D3d11ResourceDeviceTuple outputTuple = d3dImageToTuple(output);

    render(outputTuple, inputTuple);
}

void D3d11OverlayRenderer::render(const service::recorder::image::D3dImagePtr& output, ID3D11Texture2D* input, D3d11SharedContext* inputContext) {
    std::lock_guard guard(_renderMutex);
    if (!_enabled) {
        output->copyFromGpu(input);
        return;
    }

    const D3d11ResourceDeviceTuple inputTuple = loadIntoTmpInput(input, inputContext);
    const D3d11ResourceDeviceTuple outputTuple = d3dImageToTuple(output);

    render(outputTuple, inputTuple);
}

D3d11ResourceDeviceTuple D3d11OverlayRenderer::loadIntoTmpImage(service::recorder::image::D3dImagePtr& tmp, const service::recorder::image::Image& image, bool copy) {
    if (!tmp) {
        tmp = std::make_unique<service::recorder::image::D3dImage>(_context.get());
    }

    if (tmp->width() != image.width() || tmp->height() != image.height()) {
        tmp->initializeImage(image.width(), image.height(), true);
    }

    if (copy) {
        tmp->copyFromCpu(image);
    }

    return d3dImageToTuple(tmp);
}

D3d11ResourceDeviceTuple D3d11OverlayRenderer::loadIntoTmpOutput(const service::recorder::image::Image& image) {
    return loadIntoTmpImage(_tmpOutputImage, image, false);
}

D3d11ResourceDeviceTuple D3d11OverlayRenderer::loadIntoTmpInput(const service::recorder::image::Image& image) {
    return loadIntoTmpImage(_tmpInputImage, image, true);
}

D3d11ResourceDeviceTuple D3d11OverlayRenderer::loadIntoTmpInput(ID3D11Texture2D* input, D3d11SharedContext* inputContext) {
    if (!_tmpInputImage) {
        _tmpInputImage = std::make_unique<service::recorder::image::D3dImage>(_context.get());
    }

    D3D11_TEXTURE2D_DESC inputDesc;
    input->GetDesc(&inputDesc);

    if (inputDesc.Width != _tmpInputImage->width() || inputDesc.Height != _tmpInputImage->height()) {
        _tmpInputImage->initializeImage(inputDesc.Width, inputDesc.Height, true);
    }

    _tmpInputImage->copyFromSharedGpu(inputContext, input);
    return d3dImageToTuple(_tmpInputImage);
}

void D3d11OverlayRenderer::render(const D3d11ResourceDeviceTuple& output, const D3d11ResourceDeviceTuple& input) const {
    if (!_enabled) {
        THROW_ERROR("Internal D3d11OverlayRenderer must be enabled.");
    }

    // We want to get the texture to write to. Note that we assume that the input resource is made in the same context as the renderer as we always
    // want to run in the device/context of the input resource. Thus, if the output texture's context is different then we need to use a shared
    // texture. We're pretty much going to always assume that the _context is input.context and that output.context is probably different?
    // Only case where this isn't true is when the output is a CPU image - but whatever, should still be OK to use shared handles.
    SharedD3d11TextureHandlePtr shared = std::make_unique<SharedD3d11TextureHandle>(_context.get(), output.texture, true);
    ID3D11Texture2D* finalTextureOutput = shared->texture();

    D3D11_TEXTURE2D_DESC outputDesc;
    finalTextureOutput->GetDesc(&outputDesc);

    D3D11_TEXTURE2D_DESC inputDesc;
    input.texture->GetDesc(&inputDesc);

    const bool canCopy = 
        (inputDesc.Width == outputDesc.Width) &&
        (inputDesc.Height == outputDesc.Height) &&
        (inputDesc.Format == outputDesc.Format) &&
        _settings.layers.empty();

    if (canCopy) {
        auto immediate = _context->immediateContext();
        immediate.context()->CopyResource(finalTextureOutput, input.texture);
    } else {
        // Is there a way to re-use the render target?
        D3D11_RENDER_TARGET_VIEW_DESC targetDesc;
        targetDesc.Format = outputDesc.Format;
        targetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        targetDesc.Texture2D.MipSlice = 0;

        ID3D11RenderTargetView* rt = _renderer->createRenderTarget(finalTextureOutput, targetDesc);
        if (!rt) {
            THROW_ERROR("Failed to create render target.");
        }

        {
            auto texContext = input.context->immediateContext();
            _outputQuad->setTexture(input.context->device(), texContext.context(), input.texture);
            texContext.context()->Flush();
        }

        if (!_renderer->renderSceneToRenderTarget(rt)) {
            THROW_ERROR("Failed to render quad to target for overlay.");
        }

        rt->Release();
    }

    // To follow MSDN's guidelines that "If a shared texture is updated on one device ID3D11DeviceContext::Flush must be called on that device."
    // we must force the flush on the renderer's context to make sure that all the commands we put onto the
    // rendering deferred context gets run immediately.
    auto immediate = _context->immediateContext();
    immediate.context()->Flush();
}

}

#endif