#include "recorder/compositor/compositor.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"

namespace service::recorder::compositor {

Compositor::Compositor(const service::renderer::D3d11Device device, size_t width, size_t height):
    _width(width),
    _height(height)
{
    _device = device;
    _d3dContext = std::make_unique<service::renderer::D3d11SharedContext>(
        service::renderer::CONTEXT_FLAG_USE_D3D11_1,
        nullptr,
        device
    );
    _renderer = std::make_unique<service::renderer::D3d11Renderer>(_d3dContext.get());
    _renderer->initializeRenderer(_width, _height);
}

layers::ClockLayerPtr Compositor::createClockLayer() {
    if (_clockLayer) {
        LOG_WARNING("Only 1 clock layer can be created - ignoring" << std::endl);
        return nullptr;
    }

    auto layer = std::make_shared<layers::ClockLayer>();
    if (layer) {
        layer->setCallback(std::bind(&Compositor::tick, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }
    _clockLayer = layer;
    return layer;
}

void Compositor::tick(service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, size_t numFrames) {
    if (!_clockLayer) {
        return;
    }
    assert(imageContext == context().get());
    HRESULT hr = S_OK;
    // The output image needs to be the same resolution as the clock layer. Note
    // that the image we want to send will *always* come from a D3D11 texture (since we'll
    // use the WARP software rasterizer in the case of using the CPU).
    ID3D11Texture2D* texToSend = nullptr;

    D3D11_TEXTURE2D_DESC inputDesc;
    image->GetDesc(&inputDesc);

    reinitOutputTexture(image, inputDesc);
    reinitTonemapper(image, inputDesc);

    const float sx = static_cast<float>(_width) / inputDesc.Width;
    const float sy = static_cast<float>(_height) / inputDesc.Height;

    if (sx < 0.5f || sy < 0.5f) {
        _fsQuad->setResizeMode(service::renderer::TextureResizeMode::Multisample);
    } else {
        _fsQuad->setResizeMode(service::renderer::TextureResizeMode::Naive);
    }

    if (inputDesc.Width == _width && inputDesc.Height == _height && _layers.empty() && !_tonemapper) {
        // Use the clock layer directly as the image to send to the encoder.
        texToSend = image;
    } else {
        auto immediate = _d3dContext->immediateContext();

        // Update layers at the current point in time (just in case they are animated, e.g. the mouse cursor).
        // Note that the clock layer should've updated itself and prepared itself for rendering so we don't have to do that.
        const auto tm = service::recorder::encoder::AVSyncClock::now();
        for (const auto& layer: _layers) {
            layer->updateAt(tm, _renderer.get(), immediate.context());
        }

        // Set the texture every frame - this helps us ensure that the quad's texture is correct even when
        // the input texture changes/gets recreated.
        _fsQuad->setTexture(_renderer->device(), immediate.context(), image, true);

        // Do our rendering loop here. This will do a couple of things:
        //  1) Copy (additionally with a resize if necessary) to the final texture.
        //  2) Render additional layers on top.
        _renderer->renderSceneToRenderTarget(_outputRenderTarget.get());

        if (_tonemapper) {
            D3D11_TEXTURE2D_DESC hdrDesc;
            _outputTexture->GetDesc(&hdrDesc);

            // Create the SRV for the input texture.
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            srvDesc.Format = hdrDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = 1;

            // Not a big fan of creating the SRV on every frame...but I don't think it's expensive and it seems
            // like it's probably a reasonable thing to do.
            // See: https://stackoverflow.com/questions/38851449/creating-shader-resource-view-performance
            wil::com_ptr<ID3D11ShaderResourceView> hdrSrv;
            hr = _d3dContext->device()->CreateShaderResourceView(_outputTexture.get(), &srvDesc, &hdrSrv);
            if (hr != S_OK) {
                THROW_ERROR("Failed to create shader resource view.");
            }

            _renderer->prepRenderTargetForRender(immediate.context(), _tmRenderTarget.get());
            _tonemapper->SetHDRSourceTexture(hdrSrv.get());
            _tonemapper->Process(immediate.context());

            ID3D11ShaderResourceView* nullsrv[] = { nullptr };
            immediate.context()->PSSetShaderResources(0, 1, nullsrv);

            texToSend = _tmTexture.get();
        } else {
            texToSend = _outputTexture.get();
        }
    }

    // Finally send the image to the encoder. We could theoretically
    // hold the encoder mutex the whole time of the tick but I don't think
    // there's a need to optimize that. We will theoretically do extra work
    // if no encoder is set but that should be a rare state. Note that it's
    // the encoder's reponsibility for doing any color space transformations
    // and image resizing.
    if (texToSend) {
        std::lock_guard<std::mutex> guard(_encoderMutex);
        if (_activeEncoder) {
            _activeEncoder->addVideoFrame(texToSend, numFrames);
        }
    }
}

void Compositor::reinitOutputTexture(ID3D11Texture2D* image, const D3D11_TEXTURE2D_DESC& inputDesc) {
    D3D11_TEXTURE2D_DESC outputDesc = { 0 };
    if (_outputTexture) {
        _outputTexture->GetDesc(&outputDesc);
    }

    if (outputDesc.Format == inputDesc.Format) {
        return;
    }

    outputDesc.Width = _width;
    outputDesc.Height = _height;
    
    // We want to keep the input's format because we want the tonemapper to run on the lower resolution
    // image whenever possible to save compute power (no sense running HDR tonemapping on an entire 4K image if
    // the final image is going to be 720p ya feel).
    outputDesc.Format = inputDesc.Format;
    // BindFlags should have D3D11_BIND_RENDER_TARGET because we will be rendering to it.
    // Everything else should stay default since you can't CPU read/stage a render target.
    outputDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    outputDesc.Usage = D3D11_USAGE_DEFAULT;
    outputDesc.CPUAccessFlags = 0;
    outputDesc.MiscFlags = 0;

    outputDesc.MipLevels = 1;
    outputDesc.SampleDesc.Count = 1;
    outputDesc.ArraySize = 1;

    _outputTexture.attach(_renderer->createTexture2D(outputDesc));

    D3D11_RENDER_TARGET_VIEW_DESC targetDesc;
    targetDesc.Format = outputDesc.Format;
    targetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    targetDesc.Texture2D.MipSlice = 0;

    _outputRenderTarget.attach(_renderer->createRenderTarget(_outputTexture.get(), targetDesc));
}

void Compositor::reinitTonemapper(ID3D11Texture2D* image, const D3D11_TEXTURE2D_DESC& inputDesc) {
    // This technically isn't super accurate but for the purposes of our use cases where we don't really
    // expect any formats aside from DXGI_FORMAT_B8G8R8A8_UNORM and DXGI_FORMAT_R16G16B16A16_FLOAT (where
    // the latter is only the case for HDR) this is fine.
    if (inputDesc.Format != DXGI_FORMAT_B8G8R8A8_UNORM && !_tonemapper) {
        _tonemapper = std::make_unique<DirectX::ToneMapPostProcess>(_d3dContext->device());
        _tonemapper->SetOperator(DirectX::ToneMapPostProcess::Reinhard);
        _tonemapper->SetTransferFunction(DirectX::ToneMapPostProcess::SRGB);
        _tonemapper->SetExposure(-1.0f);

        D3D11_TEXTURE2D_DESC tmDesc = { 0 };
        tmDesc.Width = _width;
        tmDesc.Height = _height;
        
        // The final tonemapped image must be DXGI_FORMAT_B8G8R8A8_UNORM.
        tmDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        // BindFlags should have D3D11_BIND_RENDER_TARGET because we will be rendering to it.
        // Everything else should stay default since you can't CPU read/stage a render target.
        tmDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
        tmDesc.Usage = D3D11_USAGE_DEFAULT;
        tmDesc.CPUAccessFlags = 0;
        tmDesc.MiscFlags = 0;

        tmDesc.MipLevels = 1;
        tmDesc.SampleDesc.Count = 1;
        tmDesc.ArraySize = 1;

        _tmTexture.attach(_renderer->createTexture2D(tmDesc));

        D3D11_RENDER_TARGET_VIEW_DESC targetDesc;
        targetDesc.Format = tmDesc.Format;
        targetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        targetDesc.Texture2D.MipSlice = 0;

        _tmRenderTarget.attach(_renderer->createRenderTarget(_tmTexture.get(), targetDesc));

    } else {
        _tonemapper.reset();
        _tmTexture.reset();
        _tmRenderTarget.reset();
    }
}

void Compositor::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder) {
    std::lock_guard guard(_encoderMutex);
    _activeEncoder = encoder;
}

void Compositor::addLayer(const layers::CompositorLayerPtr& layer) {
    _layers.push_back(layer);
}

void Compositor::finalizeLayers() {
    _fsQuad = service::renderer::createFullScreenQuad(_renderer->device());
    _renderer->addModelToScene(_fsQuad);

    _clockLayer->finalizeAssetsForRenderer(_renderer.get());
    for (const auto& l: _layers) {
        l->finalizeAssetsForRenderer(_renderer.get());
    }
}

}