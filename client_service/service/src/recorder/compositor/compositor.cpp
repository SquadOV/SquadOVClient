#include "recorder/compositor/compositor.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"

namespace service::recorder::compositor {

Compositor::Compositor(const service::renderer::D3d11Device device) {
    _device = device;
    _d3dContext = std::make_unique<service::renderer::D3d11SharedContext>(
        service::renderer::CONTEXT_FLAG_USE_D3D11_1,
        nullptr,
        device
    );
    _renderer = std::make_unique<service::renderer::D3d11Renderer>(_d3dContext.get());
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
    assert(imageContext == context());
    HRESULT hr = S_OK;
    // The output image needs to be the same resolution as the clock layer. Note
    // that the image we want to send will *always* come from a D3D11 texture (since we'll
    // use the WARP software rasterizer in the case of using the CPU).
    ID3D11Texture2D* texToSend = nullptr;

    // This needs to be first since it also will set some state in the renderer which the layers may need.
    // This will also create the tonemapper if necessary. This theoretically will allocate some unnecessary
    // resources sometimes but that shoulld be fine since it won't happen very often.
    reinitOutputTexture(image);

    if (_layers.empty() && !_tonemapper) {
        // Use the clock layer directly as the image to send to the encoder.
        texToSend = image;
    } else {
        // Update layers at the current point in time (just in case they are animated, e.g. the mouse cursor).
        // Note that the clock layer should've updated itself and prepared itself for rendering so we don't have to do that.
        bool requiresRender = false;
        const auto tm = service::recorder::encoder::AVSyncClock::now();
        for (const auto& layer: _layers) {
            layer->updateAt(tm, _renderer.get());
            requiresRender |= layer->need3dRender();
        }

        // First we copy the texture to the output texture. This effectively replaces
        // the need for the renderer to do a 'clear' operation. Note that in the case
        // of an HDR input texture, the tonemapping operation does the copy for us.
        auto immediate = _d3dContext->immediateContext();
        if (!_tonemapper) {
            immediate.context()->CopyResource(_outputTexture.get(), image);
        } else {
            D3D11_TEXTURE2D_DESC hdrDesc;
            image->GetDesc(&hdrDesc);

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
            hr = _d3dContext->device()->CreateShaderResourceView(image, &srvDesc, &hdrSrv);
            if (hr != S_OK) {
                THROW_ERROR("Failed to create shader resource view.");
            }

            _renderer->prepRenderTargetForRender(immediate.context(), _outputRenderTarget.get());
            _tonemapper->SetHDRSourceTexture(hdrSrv.get());
            _tonemapper->Process(immediate.context());

            ID3D11ShaderResourceView* nullsrv[] = { nullptr };
            immediate.context()->PSSetShaderResources(0, 1, nullsrv);
        }

        // Do our rendering loop here. Assume the scene is already setup in the finalizeLayers
        // step so the only thing to do here is to actually do the 'render'. Note that if no
        // layers actually need the renderer then we can just use a regular CopyResource to do the
        // job of copying the base texture to the output texture instead.
        if (requiresRender) {
            _renderer->renderSceneToRenderTarget(_outputRenderTarget.get());
        }

        if (!_layers.empty()) {
            HDC hdc;
            hr = _outputSurface->GetDC(false, &hdc);
            if (hr == S_OK) {
                // Do a final custom rendering loop here on the output texture just for things that use GDI instead.
                try {
                    for (const auto& layer: _layers) {
                        layer->customRender(_outputTexture.get(), _outputSurface.get(), hdc);
                    }
                } catch (std::exception& ex) {
                    LOG_WARNING("Failure while doing custom render with GDI: " << ex.what() << std::endl);
                }

                _outputSurface->ReleaseDC(nullptr);
            } else {
                LOG_ERROR("Failed to get DC for composition...: " << hr << std::endl);
            }
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

void Compositor::reinitOutputTexture(ID3D11Texture2D* input) {
    // We assume that the input texture must match the specs of the output texture.
    D3D11_TEXTURE2D_DESC inputDesc;
    input->GetDesc(&inputDesc);

    D3D11_TEXTURE2D_DESC outputDesc = { 0 };
    if (_outputTexture) {
        _outputTexture->GetDesc(&outputDesc);
    }

    if (inputDesc.Width == outputDesc.Width 
        && inputDesc.Height == outputDesc.Height) {
        return;
    }

    LOG_INFO("Initializing Compositor Output Texture: " << inputDesc.Width << "x" << inputDesc.Height << " [" << inputDesc.Format << "]" << std::endl);
    outputDesc = inputDesc;

    // Need to manually set the format since the input format could be floating point
    // and we don't want that.
    outputDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    // BindFlags should have D3D11_BIND_RENDER_TARGET because we will be rendering to it.
    // MiscFlags should have D3D11_RESOURCE_MISC_GDI_COMPATIBLE since we use some GDI rendering functions (e.g. the mouse cursor).
    // Everything else should stay default since you can't CPU read/stage a render target.
    outputDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    outputDesc.Usage = D3D11_USAGE_DEFAULT;
    outputDesc.CPUAccessFlags = 0;
    outputDesc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

    // These two must be set this way to be compatible with using the D3D11_RESOURCE_MISC_GDI_COMPATIBLE flag.
    outputDesc.MipLevels = 1;
    outputDesc.SampleDesc.Count = 1;

    _outputTexture.attach(_renderer->createTexture2D(outputDesc));

    HRESULT hr = _outputTexture->QueryInterface(__uuidof(IDXGISurface1), (void**)&_outputSurface);        
    if (hr != S_OK) {
        THROW_ERROR("Failed to get IDXGISurface1 for compositor" << hr);
        return;
    }

    D3D11_RENDER_TARGET_VIEW_DESC targetDesc;
    targetDesc.Format = outputDesc.Format;
    targetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    targetDesc.Texture2D.MipSlice = 0;

    _outputRenderTarget.attach(_renderer->createRenderTarget(_outputTexture.get(), targetDesc));
    // This technically isn't super accurate but for the purposes of our use cases where we don't really
    // expect any formats aside from DXGI_FORMAT_B8G8R8A8_UNORM and DXGI_FORMAT_R16G16B16A16_FLOAT (where
    // the latter is only the case for HDR) this is fine.
    if (inputDesc.Format != DXGI_FORMAT_B8G8R8A8_UNORM) {
        _tonemapper = std::make_unique<DirectX::ToneMapPostProcess>(_d3dContext->device());
        _tonemapper->SetOperator(DirectX::ToneMapPostProcess::Reinhard);
        _tonemapper->SetTransferFunction(DirectX::ToneMapPostProcess::SRGB);
        _tonemapper->SetExposure(-1.0f);
    } else {
        _tonemapper.reset();
    }
    _renderer->initializeRenderer(outputDesc.Width, outputDesc.Height);
}

void Compositor::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder) {
    std::lock_guard guard(_encoderMutex);
    _activeEncoder = encoder;
}

void Compositor::addLayer(const layers::CompositorLayerPtr& layer) {
    _layers.push_back(layer);
}

void Compositor::finalizeLayers() {
    _clockLayer->finalizeAssetsForRenderer(_renderer.get());
    for (const auto& l: _layers) {
        l->finalizeAssetsForRenderer(_renderer.get());
    }
}

}