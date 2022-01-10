#include "recorder/image/d3d_image.h"
#include "shared/errors/error.h"

namespace service::recorder::image {

D3dImageRendererBundle::D3dImageRendererBundle(service::renderer::D3d11SharedContext* inputContext, service::renderer::D3d11SharedContext* outputContext, ID3D11Texture2D* output, bool sameDevice):
    _shared(inputContext),
    _outputTexture(output),
    _sameDevice(sameDevice) {

    _context = _shared->deferredContext();
    _renderer = std::make_unique<service::renderer::D3d11Renderer>(_shared);
    _fullscreenQuad = service::renderer::createFullScreenQuad(_shared->device());
    _renderer->addModelToScene(_fullscreenQuad);

    D3D11_TEXTURE2D_DESC desc;
    output->GetDesc(&desc);
    _outputTexture->AddRef();

    _renderer->initializeRenderer(desc.Width, desc.Height);

    D3D11_RENDER_TARGET_VIEW_DESC targetDesc;
    targetDesc.Format = desc.Format;
    targetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    targetDesc.Texture2D.MipSlice = 0;

    if (sameDevice) {
        _renderTarget = _renderer->createRenderTarget(output, targetDesc);
    } else {
        _sharedOutput = std::make_unique<service::renderer::SharedD3d11TextureHandle>(_shared, output, true);
        _renderTarget = _renderer->createRenderTarget(_sharedOutput->texture(), targetDesc);
    }

    if (!_renderTarget) {
        THROW_ERROR("Failed to create render target.");
    }
}

D3dImageRendererBundle::~D3dImageRendererBundle() {
    if (_renderTarget) {
        _renderTarget->Release();
        _renderTarget = nullptr;
    }

    if (_outputTexture) {
        _outputTexture->Release();
        _outputTexture = nullptr;
    }

    if (_context) {
        _context->Release();
        _context = nullptr;
    }
}

ID3D11Texture2D* D3dImageRendererBundle::rawOutputTexture() const {
    if (_sharedOutput) {
        return _sharedOutput->texture();
    }
    return _outputTexture;
}

void D3dImageRendererBundle::render(ID3D11Texture2D* image, DXGI_MODE_ROTATION rotation) {
    if (!_renderTarget) {
        LOG_WARNING("No render target to render to...ignoring." << std::endl);
        return;
    }

    // First check to see if we can just do a regular old copy.
    // For this to happen we need resolution and format to be the same as well
    // as the input specified rotation to be identity.
    D3D11_TEXTURE2D_DESC imageDesc;
    image->GetDesc(&imageDesc);

    D3D11_TEXTURE2D_DESC textureDesc;
    _outputTexture->GetDesc(&textureDesc);

    const bool canCopy = 
        !_forceNoCopy &&
        (imageDesc.Width == textureDesc.Width) &&
        (imageDesc.Height == textureDesc.Height) &&
        (imageDesc.Format == textureDesc.Format) &&
        (rotation == DXGI_MODE_ROTATION_IDENTITY || rotation == DXGI_MODE_ROTATION_UNSPECIFIED);

    if (canCopy) {
        auto immediate = _shared->immediateContext();
        immediate.context()->CopyResource(rawOutputTexture(), image);
    } else {
        _fullscreenQuad->setTexture(_shared->device(), _context, image);

        switch (rotation) {
        case DXGI_MODE_ROTATION_ROTATE90:
            _fullscreenQuad->setZRotation(90.f);
            break;
        case DXGI_MODE_ROTATION_ROTATE180:
            _fullscreenQuad->setZRotation(180.f);
            break;
        case DXGI_MODE_ROTATION_ROTATE270:
            _fullscreenQuad->setZRotation(270.f);
            break;
        default:
            _fullscreenQuad->setZRotation(0.f);
            break;
        }

        // Need to do a pre-emptive execute here since we use a separate deferred context
        // from the renderer's deferred context to load up the input texture.
        _shared->execute(_context);

        if (!_renderer->renderSceneToRenderTarget(_renderTarget)) {
            LOG_ERROR("Failed to render textured quad to render target..." << std::endl);
            return;
        }
    }

    if (!_sameDevice) {
        auto immediate = _shared->immediateContext();
        immediate.context()->Flush();
    }
}

D3dImage::D3dImage(service::renderer::D3d11SharedContext* shared):
    _shared(shared) {
}

D3dImage::~D3dImage() {
    releaseTextureIfExists();
    releaseOutputTextureIfExists();
    releaseInputTextureIfExists();
}

D3dImageRendererBundle* D3dImage::getRendererBundle(service::renderer::D3d11SharedContext* context) {
    auto it = _renderBundles.find(context);
    if (it != _renderBundles.end()) {
        return it->second.get();
    } else {
        auto bundle = std::make_unique<D3dImageRendererBundle>(context, _shared, _hwTexture, context == _shared);
        auto* ptr = bundle.get();
        _renderBundles[context] = std::move(bundle);
        return ptr;
    }
    return nullptr;
}

void D3dImage::releaseTextureIfExists() {
    if (_hwTexture) {
        _hwTexture->Release();
        _hwTexture = nullptr;
    }
}

void D3dImage::initializeImage(size_t width, size_t height, bool shared, DXGI_FORMAT format) {
    releaseTextureIfExists();

    _width = width;
    _height = height;
    _format = format;

    D3D11_TEXTURE2D_DESC sharedDesc = { 0 };
    sharedDesc.Width = width;
    sharedDesc.Height = height;
    sharedDesc.MipLevels = 1;
    sharedDesc.ArraySize = 1;
    sharedDesc.Format = format;
    sharedDesc.SampleDesc.Count = 1;
    sharedDesc.Usage = D3D11_USAGE_DEFAULT;
    sharedDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    sharedDesc.CPUAccessFlags = 0;
    sharedDesc.MiscFlags = shared ? D3D11_RESOURCE_MISC_SHARED : 0;

    HRESULT hr = _shared->device()->CreateTexture2D(&sharedDesc, nullptr, &_hwTexture);
    if (hr != S_OK || !_hwTexture) {
        THROW_ERROR("Failed to create D3d image.");
    }

    // Need to release the renderer bundles if the size changes too as we're changing the
    // size of the output image.
    _renderBundles.clear();

    _init = true;
}

void D3dImage::copyFromCpu(const Image& image) {
    refreshInputTexture(image.width(), image.height(), image.format());

    D3D11_BOX box;
    box.left = 0;
    box.right = image.width();
    box.top = 0;
    box.bottom = image.height();
    box.front = 0;
    box.back = 1;

    {
        auto immediate = _shared->immediateContext();
        immediate.context()->UpdateSubresource(_inputTexture, 0, &box, image.buffer(), image.numBytesPerRow(), 0);
    }

    copyFromGpu(_inputTexture);
}

void D3dImage::copyFromGpu(ID3D11Texture2D* image, DXGI_MODE_ROTATION rotation) {
   copyFromSharedGpu(_shared, image, rotation);
}

void D3dImage::copyFromSharedGpu(service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, DXGI_MODE_ROTATION rotation) {
    // First check to see if we can just do a regular old copy.
    // For this to happen we need resolution and format to be the same as well
    // as the input specified rotation to be identity.
    D3D11_TEXTURE2D_DESC imageDesc;
    image->GetDesc(&imageDesc);

    D3D11_TEXTURE2D_DESC textureDesc;
    _hwTexture->GetDesc(&textureDesc);

    // We assume that the rendering/GPU operation must *always* happen on the input context.
    auto* bundle = getRendererBundle(imageContext);
    if (!bundle) {
        THROW_ERROR("Failed to obtain renderer bundle.");
    }
    bundle->render(image, rotation);
}

ID3D11Texture2D* D3dImage::createStagingTexture(size_t width, size_t height, bool forCpu, DXGI_FORMAT format) {
    D3D11_TEXTURE2D_DESC sharedDesc = { 0 };
    sharedDesc.Width = width;
    sharedDesc.Height = height;
    sharedDesc.MipLevels = 1;
    sharedDesc.ArraySize = 1;
    sharedDesc.Format = format;
    sharedDesc.SampleDesc.Count = 1;
    sharedDesc.Usage = forCpu ? D3D11_USAGE_STAGING : D3D11_USAGE_DEFAULT;
    sharedDesc.BindFlags = 0;
    sharedDesc.CPUAccessFlags = forCpu ? D3D11_CPU_ACCESS_READ : 0;
    sharedDesc.MiscFlags = 0;

    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = _shared->device()->CreateTexture2D(&sharedDesc, nullptr, &texture);
    if (hr != S_OK) {
        THROW_ERROR("Failed to create staging texture: " << hr);
    }
    return texture;
}

void D3dImage::releaseInputTextureIfExists() {
    if (_inputTexture) {
        _inputTexture->Release();
        _inputTexture = nullptr;
    }
}

void D3dImage::refreshInputTexture(size_t width, size_t height, DXGI_FORMAT format) {
    bool needsRefresh = true;
    if (_inputTexture) {
        D3D11_TEXTURE2D_DESC desc;
        _inputTexture->GetDesc(&desc);
        needsRefresh = (desc.Width != width || desc.Height != height || desc.Format != format);
    }

    if (!needsRefresh) {
        return;
    }

    releaseInputTextureIfExists();
    _inputTexture = createStagingTexture(width, height, false, format);
}

void D3dImage::releaseOutputTextureIfExists() {
    if (_outputTexture) {
        _outputTexture->Release();
        _outputTexture = nullptr;
    }
}

void D3dImage::refreshOutputTexture(size_t width, size_t height) {
    bool needsRefresh = true;
    if (_outputTexture) {
        D3D11_TEXTURE2D_DESC desc;
        _outputTexture->GetDesc(&desc);
        needsRefresh = (desc.Width != width || desc.Height != height);
    }

    if (!needsRefresh) {
        return;
    }

    releaseOutputTextureIfExists();
    _outputTexture = createStagingTexture(width, height, true);
}

Image D3dImage::cpuImage() {
    Image retImage;
    retImage.initializeImage(_width, _height);
    refreshOutputTexture(_width, _height);

    auto immediate = _shared->immediateContext();
    immediate.context()->CopyResource(_outputTexture, _hwTexture);

    retImage.loadFromD3d11Texture(immediate.context(), _outputTexture);
    return retImage;
}

}